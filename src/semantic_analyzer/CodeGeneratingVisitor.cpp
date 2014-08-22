#include "CodeGeneratingVisitor.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "ArithmeticExpression.h"
#include "ArrayAccess.h"
#include "ArrayDeclaration.h"
#include "AssignmentExpression.h"
#include "BasicType.h"
#include "BitwiseExpression.h"
#include "ComparisonExpression.h"
#include "ExpressionList.h"
#include "ForLoopHeader.h"
#include "FunctionCall.h"
#include "FunctionDeclaration.h"
#include "FunctionDefinition.h"
#include "IfElseStatement.h"
#include "IfStatement.h"
#include "IOStatement.h"
#include "LogicalAndExpression.h"
#include "LogicalOrExpression.h"
#include "LoopStatement.h"
#include "ParameterDeclaration.h"
#include "PointerCast.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "ShiftExpression.h"
#include "Term.h"
#include "TerminalSymbol.h"
#include "TypeCast.h"
#include "UnaryExpression.h"
#include "VariableDeclaration.h"
#include "VariableDefinition.h"
#include "WhileLoopHeader.h"

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

void CodeGeneratingVisitor::visit(TypeSpecifier&) {
}

void CodeGeneratingVisitor::visit(ParameterList& parameterList) {
    for (auto& parameter : parameterList.getDeclaredParameters()) {
        parameter->accept(*this);
    }
}

void CodeGeneratingVisitor::visit(AssignmentExpressionList& expressions) {
    for (auto& expression : expressions.getExpressions()) {
        expression->accept(*this);
    }
}

void CodeGeneratingVisitor::visit(DeclarationList& declarations) {
    for (auto& declaration : declarations.getDeclarations()) {
        declaration->accept(*this);
    }
}

void CodeGeneratingVisitor::visit(ArrayAccess& arrayAccess) {
    arrayAccess.postfixExpression->accept(*this);
    arrayAccess.subscriptExpression->accept(*this);

    auto offset = arrayAccess.subscriptExpression->getResultHolder();
    quadruples.push_back( { INDEX, arrayAccess.postfixExpression->getResultHolder(), offset, arrayAccess.getResultHolder() });
    quadruples.push_back( { INDEX_ADDR, arrayAccess.postfixExpression->getResultHolder(), offset, arrayAccess.getLvalue() });
}

void CodeGeneratingVisitor::visit(FunctionCall& functionCall) {
    functionCall.callExpression->accept(*this);
    functionCall.argumentList->accept(*this);

    for (auto& expression : functionCall.argumentList->getExpressions()) {
        quadruples.push_back( { PARAM, expression->getResultHolder(), nullptr, nullptr });
    }

    quadruples.push_back( { CALL, functionCall.callExpression->getResultHolder(), nullptr, nullptr });
    if (functionCall.getTypeInfo().getBasicType() != BasicType::VOID || functionCall.getTypeInfo().getExtendedType() != "") {
        quadruples.push_back( { RETRIEVE, functionCall.getResultHolder(), nullptr, nullptr });
    }
}

void CodeGeneratingVisitor::visit(Term& term) {
    if (term.term.type != "id") {
        quadruples.push_back( { term.term.value, term.getResultHolder() });
    }
}

void CodeGeneratingVisitor::visit(PostfixExpression& expression) {
    expression.postfixExpression->accept(*this);

    if (expression.postfixOperator.value == "++") {
        quadruples.push_back( { INC, expression.getResultHolder(), nullptr, expression.getResultHolder() });
    } else if (expression.postfixOperator.value == "--") {
        quadruples.push_back( { DEC, expression.getResultHolder(), nullptr, expression.getResultHolder() });
    }
}

void CodeGeneratingVisitor::visit(PrefixExpression& expression) {
    if (expression.incrementOperator.value == "++") {
        quadruples.push_back( { INC, expression.getResultHolder(), nullptr, expression.getResultHolder() });
    } else if (expression.incrementOperator.value == "--") {
        quadruples.push_back( { DEC, expression.getResultHolder(), nullptr, expression.getResultHolder() });
    }

    expression.unaryExpression->accept(*this);  // increment before evaluating the expression
}

void CodeGeneratingVisitor::visit(UnaryExpression& expression) {
    expression.castExpression->accept(*this);

    switch (expression.unaryOperator.value.at(0)) {
    case '&':
        quadruples.push_back( { ADDR, expression.castExpression->getResultHolder(), nullptr, expression.getResultHolder() });
        expression.castExpression->accept(*this);
        break;
    case '*':
        quadruples.push_back( { DEREF, expression.castExpression->getResultHolder(), nullptr, expression.getResultHolder() });
        expression.castExpression->accept(*this);
        break;
    case '+':
        expression.castExpression->accept(*this);
        break;
    case '-':
        expression.castExpression->accept(*this);
        quadruples.push_back( { UMINUS, expression.castExpression->getResultHolder(), nullptr, expression.getResultHolder() });
        break;
    case '!':
        expression.castExpression->accept(*this);
        quadruples.push_back( { CMP, expression.castExpression->getResultHolder(), 0, expression.getResultHolder() });
        quadruples.push_back( { JE, expression.getTruthyLabel(), nullptr, nullptr });
        quadruples.push_back( { "0", expression.getResultHolder() });
        quadruples.push_back( { GOTO, expression.getFalsyLabel(), nullptr, nullptr });
        quadruples.push_back( { LABEL, expression.getTruthyLabel(), nullptr, nullptr });
        quadruples.push_back( { "1", expression.getResultHolder() });
        quadruples.push_back( { LABEL, expression.getFalsyLabel(), nullptr, nullptr });
        break;
    default:
        throw std::runtime_error { "Unidentified increment operator: " + expression.unaryOperator.value };
    }
}

void CodeGeneratingVisitor::visit(TypeCast& expression) {
    expression.castExpression->accept(*this);

    quadruples.push_back( { ASSIGN, expression.castExpression->getResultHolder(), nullptr, expression.getResultHolder() });
}

void CodeGeneratingVisitor::visit(PointerCast& expression) {
    expression.pointer->accept(*this);
    expression.castExpression->accept(*this);

    quadruples.push_back( { ASSIGN, expression.castExpression->getResultHolder(), nullptr, expression.getResultHolder() });
}

void CodeGeneratingVisitor::visit(ArithmeticExpression& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);

    auto leftOperand = expression.leftHandSide->getResultHolder();
    auto rightOperand = expression.rightHandSide->getResultHolder();
    auto resultHolder = expression.getResultHolder();
    switch (expression.arithmeticOperator.value.at(0)) {
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
        throw std::runtime_error { "unidentified arithmetic operator: " + expression.arithmeticOperator.type };
    }
}

void CodeGeneratingVisitor::visit(ShiftExpression& expression) {
    expression.shiftExpression->accept(*this);
    expression.additionExpression->accept(*this);

    auto leftOperand = expression.shiftExpression->getResultHolder();
    auto rightOperand = expression.additionExpression->getResultHolder();
    auto resultHolder = expression.getResultHolder();
    switch (expression.shiftOperator.value.at(0)) {
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

void CodeGeneratingVisitor::visit(ComparisonExpression& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);

    auto leftOperand = expression.leftHandSide->getResultHolder();
    auto rightOperand = expression.rightHandSide->getResultHolder();
    quadruples.push_back( { CMP, leftOperand, rightOperand, nullptr });

    auto truthyLabel = expression.getTruthyLabel();
    if (expression.comparisonOperator.value == ">") {
        quadruples.push_back( { JA, truthyLabel, nullptr, nullptr });
    } else if (expression.comparisonOperator.value == "<") {
        quadruples.push_back( { JB, truthyLabel, nullptr, nullptr });
    } else if (expression.comparisonOperator.value == "<=") {
        quadruples.push_back( { JBE, truthyLabel, nullptr, nullptr });
    } else if (expression.comparisonOperator.value == ">=") {
        quadruples.push_back( { JAE, truthyLabel, nullptr, nullptr });
    } else if (expression.comparisonOperator.value == "==") {
        quadruples.push_back( { JE, truthyLabel, nullptr, nullptr });
    } else if (expression.comparisonOperator.value == "!=") {
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

void CodeGeneratingVisitor::visit(BitwiseExpression& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);

    oper quadrupleOperator;
    switch (expression.bitwiseOperator.type.at(0)) {
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
        throw std::runtime_error { "no semantic actions defined for bitwise operator: " + expression.bitwiseOperator.type };
    }
    auto leftOperand = expression.leftHandSide->getResultHolder();
    auto rightOperand = expression.rightHandSide->getResultHolder();
    quadruples.push_back( { quadrupleOperator, leftOperand, rightOperand, expression.getResultHolder() });
}

void CodeGeneratingVisitor::visit(LogicalAndExpression& expression) {
    expression.leftHandSide->accept(*this);

    auto leftOperand = expression.leftHandSide->getResultHolder();

    quadruples.push_back( { "0", expression.getResultHolder() });
    quadruples.push_back( { CMP, leftOperand, 0, nullptr });
    quadruples.push_back( { JE, expression.getExitLabel(), nullptr, nullptr });

    expression.rightHandSide->accept(*this);

    auto rightOperand = expression.rightHandSide->getResultHolder();
    quadruples.push_back( { CMP, rightOperand, 0, nullptr });
    quadruples.push_back( { JE, expression.getExitLabel(), nullptr, nullptr });
    quadruples.push_back( { "1", expression.getResultHolder() });

    quadruples.push_back( { LABEL, expression.getExitLabel(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(LogicalOrExpression& expression) {
    expression.leftHandSide->accept(*this);

    auto leftOperand = expression.leftHandSide->getResultHolder();
    quadruples.push_back( { "1", expression.getResultHolder() });
    quadruples.push_back( { CMP, leftOperand, 0, nullptr });
    quadruples.push_back( { JNE, expression.getExitLabel(), nullptr, nullptr });

    expression.rightHandSide->accept(*this);

    auto rightOperand = expression.rightHandSide->getResultHolder();
    quadruples.push_back( { CMP, rightOperand, 0, nullptr });
    quadruples.push_back( { JNE, expression.getExitLabel(), nullptr, nullptr });
    quadruples.push_back( { "0", expression.getResultHolder() });

    quadruples.push_back( { LABEL, expression.getExitLabel(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(AssignmentExpression& expression) {
    expression.leftHandSide->accept(*this);
    SymbolEntry* dereferencedLocation {nullptr};
    if (quadruples.back().getOp() == DEREF) {
        dereferencedLocation = quadruples.back().getArg1();
        quadruples.pop_back();
    }
    expression.rightHandSide->accept(*this);

    auto valueToAssign = expression.rightHandSide->getResultHolder();
    auto resultPlace = expression.getResultHolder();
    auto assignmentOperator = expression.assignmentOperator;
    if (assignmentOperator.value == "+=")
        quadruples.push_back( { ADD, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator.value == "-=")
        quadruples.push_back( { SUB, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator.value == "*=")
        quadruples.push_back( { MUL, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator.value == "/=")
        quadruples.push_back( { DIV, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator.value == "%=")
        quadruples.push_back( { MOD, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator.value == "&=")
        quadruples.push_back( { AND, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator.value == "^=")
        quadruples.push_back( { XOR, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator.value == "|=")
        quadruples.push_back( { OR, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator.value == "<<=")
        quadruples.push_back( { SHL, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator.value == ">>=")
        quadruples.push_back( { SHR, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator.value == "=") {
        if (dereferencedLocation) {
            quadruples.push_back({ DEREF_LVAL, valueToAssign, nullptr, dereferencedLocation});
        } else {
            quadruples.push_back( { ASSIGN, valueToAssign, nullptr, resultPlace });
        }
    } else {
        throw std::runtime_error { "unidentified assignment operator: " + assignmentOperator.value };
    }
}

void CodeGeneratingVisitor::visit(ExpressionList& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);
}

void CodeGeneratingVisitor::visit(JumpStatement& statement) {
    throw std::runtime_error { "not implemented" };
}

void CodeGeneratingVisitor::visit(ReturnStatement& statement) {
    statement.returnExpression->accept(*this);
    quadruples.push_back( { RETURN, statement.returnExpression->getResultHolder(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(IOStatement& statement) {
    statement.expression->accept(*this);
    if (statement.ioKeyword.value == "output") {
        quadruples.push_back( { OUT, statement.expression->getResultHolder(), nullptr, nullptr });
    } else if (statement.ioKeyword.value == "input") {
        quadruples.push_back( { IN, statement.expression->getResultHolder(), nullptr, nullptr });
    } else {
        throw std::runtime_error { "bad IO statement: " + statement.ioKeyword.type };
    }
}

void CodeGeneratingVisitor::visit(IfStatement& statement) {
    statement.testExpression->accept(*this);
    quadruples.push_back( { CMP, statement.testExpression->getResultHolder(), 0, nullptr });
    quadruples.push_back( { JE, statement.getFalsyLabel(), nullptr, nullptr });

    statement.body->accept(*this);
    quadruples.push_back( { LABEL, statement.getFalsyLabel(), nullptr, nullptr });

}

void CodeGeneratingVisitor::visit(IfElseStatement& statement) {
    statement.testExpression->accept(*this);

    quadruples.push_back( { CMP, statement.testExpression->getResultHolder(), 0, nullptr });
    quadruples.push_back( { JE, statement.getFalsyLabel(), nullptr, nullptr });

    statement.truthyBody->accept(*this);
    quadruples.push_back( { GOTO, statement.getExitLabel(), nullptr, nullptr });
    quadruples.push_back( { LABEL, statement.getFalsyLabel(), nullptr, nullptr });

    statement.falsyBody->accept(*this);
    quadruples.push_back( { LABEL, statement.getExitLabel(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(LoopStatement& loop) {
    loop.header->accept(*this);
    loop.body->accept(*this);
    // FIXME:
    if (loop.header->increment) {
        loop.header->increment->accept(*this);
    }

    quadruples.push_back( { GOTO, loop.header->getLoopEntry(), nullptr, nullptr });
    quadruples.push_back( { LABEL, loop.header->getLoopExit(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(ForLoopHeader& loopHeader) {
    loopHeader.initialization->accept(*this);

    quadruples.push_back( { LABEL, loopHeader.getLoopEntry(), nullptr, nullptr });
    loopHeader.clause->accept(*this);
    quadruples.push_back( { CMP, loopHeader.clause->getResultHolder(), 0, nullptr });
    quadruples.push_back( { JE, loopHeader.getLoopExit(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(WhileLoopHeader& loopHeader) {
    quadruples.push_back( { LABEL, loopHeader.getLoopEntry(), nullptr, nullptr });
    loopHeader.clause->accept(*this);
    quadruples.push_back( { CMP, loopHeader.clause->getResultHolder(), 0, nullptr });
    quadruples.push_back( { JE, loopHeader.getLoopExit(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(Pointer&) {
}

void CodeGeneratingVisitor::visit(Identifier&) {
}

void CodeGeneratingVisitor::visit(FunctionDeclaration& declaration) {
    declaration.parameterList->accept(*this);
}

void CodeGeneratingVisitor::visit(ArrayDeclaration& declaration) {
    declaration.subscriptExpression->accept(*this);
    throw std::runtime_error { "not implemented" };
}

void CodeGeneratingVisitor::visit(ParameterDeclaration& parameter) {
    parameter.declaration->accept(*this);
}

void CodeGeneratingVisitor::visit(FunctionDefinition& function) {
    function.declaration->accept(*this);

    auto functionHolder = function.declaration->getHolder();
    quadruples.push_back( { PROC, functionHolder, nullptr, nullptr });
    function.body->accept(*this);
    quadruples.push_back( { ENDPROC, functionHolder, nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(VariableDeclaration& declaration) {
    declaration.declaredVariables->accept(*this);
}

void CodeGeneratingVisitor::visit(VariableDefinition& definition) {
    definition.declaration->accept(*this);
    definition.initializerExpression->accept(*this);

    auto& declaredVariables = definition.declaration->declaredVariables->getDeclarations();
    auto& lastVariableInDeclaration = declaredVariables.at(declaredVariables.size() - 1);
    quadruples.push_back( { ASSIGN, definition.initializerExpression->getResultHolder(), nullptr, lastVariableInDeclaration->getHolder() });
}

void CodeGeneratingVisitor::visit(Block& block) {
    quadruples.push_back( { SCOPE, nullptr, nullptr, nullptr });
    for (auto& statement : block.getChildren()) {
        statement->accept(*this);
    }
    quadruples.push_back( { ENDSCOPE, nullptr, nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(ListCarrier& listCarrier) {
    for (const auto& child : listCarrier.getChildren()) {
        child->accept(*this);
    }
}

void CodeGeneratingVisitor::visit(TranslationUnit& translationUnit) {
    for (const auto& child : translationUnit.getChildren()) {
        child->accept(*this);
    }
}

std::vector<Quadruple> CodeGeneratingVisitor::getQuadruples() const {
    return quadruples;
}

} /* namespace semantic_analyzer */

