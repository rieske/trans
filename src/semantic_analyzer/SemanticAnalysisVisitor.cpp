#include "SemanticAnalysisVisitor.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "../code_generator/symbol_entry.h"
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
#include "FunctionDefinition.h"
#include "IfElseStatement.h"
#include "IfStatement.h"
#include "IOStatement.h"
#include "LogicalExpression.h"
#include "LogicalOrExpression.h"
#include "LogicalAndExpression.h"
#include "LoopStatement.h"
#include "ParameterDeclaration.h"
#include "PointerCast.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "ShiftExpression.h"
#include "TerminalSymbol.h"
#include "TypeCast.h"
#include "TypeSpecifier.h"
#include "UnaryExpression.h"
#include "VariableDeclaration.h"
#include "VariableDefinition.h"

namespace semantic_analyzer {

SemanticAnalysisVisitor::SemanticAnalysisVisitor() :
        symbolTable { new SymbolTable { } },
        currentScope { symbolTable.get() } {
}

SemanticAnalysisVisitor::~SemanticAnalysisVisitor() {
}

void SemanticAnalysisVisitor::visit(TypeSpecifier&) {
}

void SemanticAnalysisVisitor::visit(ParameterList& parameterList) {
    for (auto& declaredParameter : parameterList.getDeclaredParameters()) {
        declaredParameter->accept(*this);
    }
}

void SemanticAnalysisVisitor::visit(AssignmentExpressionList& expressions) {
    for (auto& expression : expressions.getExpressions()) {
        expression->accept(*this);
    }
}

void SemanticAnalysisVisitor::visit(DeclarationList& declarations) {
    for (auto& declaration : declarations.getDeclarations()) {
        declaration->accept(*this);
    }
}

void SemanticAnalysisVisitor::visit(ArrayAccess& arrayAccess) {
    arrayAccess.postfixExpression->accept(*this);
    arrayAccess.subscriptExpression->accept(*this);

    auto extendedType = arrayAccess.getExtendedType();
    if (extendedType.size() && (extendedType.at(0) == 'p' || extendedType.at(0) == 'a')) {
        extendedType = extendedType.substr(1, extendedType.size());
        arrayAccess.setLvalue(currentScope->newTemp(arrayAccess.getBasicType(), extendedType));
        arrayAccess.setResultHolder(currentScope->newTemp(arrayAccess.getBasicType(), extendedType));
    } else {
        // FIXME: line number
        error("invalid type for operator[]\n", 0);
    }
}

void SemanticAnalysisVisitor::visit(FunctionCall& functionCall) {
    functionCall.callExpression->accept(*this);
    functionCall.argumentList->accept(*this);

    auto resultPlace = functionCall.callExpression->getResultHolder();
    auto& arguments = functionCall.argumentList->getExpressions();
    if (arguments.size() != resultPlace->getParamCount()) {
        // FIXME: line number
        error("no match for function " + resultPlace->getName(), 0);
    } else {
        vector<SymbolEntry *> declaredArguments = resultPlace->getParams();
        for (size_t i { 0 }; i < arguments.size(); ++i) {
            SymbolEntry *param = arguments.at(i)->getResultHolder();
            string check;
            if ("ok" != (check = currentScope->typeCheck(declaredArguments.at(i), param))) {
                // FIXME: line number
                error(check, 0);
            }
        }
        auto basicType = resultPlace->getBasicType();
        auto extendedType = resultPlace->getExtendedType();
        if (basicType != BasicType::VOID || extendedType != "") {
            extendedType = extendedType.substr(0, extendedType.size() - 1);
            resultPlace = currentScope->newTemp(basicType, extendedType);
        }
        functionCall.setResultHolder(resultPlace);
    }
}

void SemanticAnalysisVisitor::visit(Term& term) {
    if (term.term.type == "id") {
        if (currentScope->hasSymbol(term.term.value)) {
            term.setResultHolder(currentScope->lookup(term.term.value));
        } else {
            error("symbol `" + term.term.value + "` is not defined", term.term.line);
        }
    } else {
        term.setResultHolder(currentScope->newTemp(term.getBasicType(), term.getExtendedType()));
    }
}

void SemanticAnalysisVisitor::visit(PostfixExpression& expression) {
    expression.postfixExpression->accept(*this);
    if (!expression.postfixExpression->isLval()) {
        error("lvalue required as increment operand", expression.postfixOperator.line);
    }
}

void SemanticAnalysisVisitor::visit(PrefixExpression& expression) {
    expression.unaryExpression->accept(*this);
    if (!expression.unaryExpression->isLval()) {
        error("lvalue required as increment operand", expression.incrementOperator.line);
    }
}

void SemanticAnalysisVisitor::visit(UnaryExpression& expression) {
    expression.castExpression->accept(*this);
    switch (expression.unaryOperator.value.at(0)) {
    case '&':
        expression.setResultHolder(currentScope->newTemp(expression.getBasicType(), "p" + expression.getExtendedType()));
        break;
    case '*':
        if (expression.getExtendedType().size() && expression.getExtendedType().at(0) == 'p') {
            expression.setResultHolder(
                    currentScope->newTemp(expression.getBasicType(),
                            expression.getExtendedType().substr(1, expression.getExtendedType().size())));
        } else {
            error("invalid type argument of ‘unary *’ " + expression.getExtendedType(), expression.unaryOperator.line);
        }
        break;
    case '+':
        break;
    case '-':
        expression.setResultHolder(currentScope->newTemp(expression.getBasicType(), expression.getExtendedType()));
        break;
    case '!':
        expression.setResultHolder(currentScope->newTemp(BasicType::INTEGER, ""));
        expression.setTruthyLabel(currentScope->newLabel());
        expression.setFalsyLabel(currentScope->newLabel());
        break;
    default:
        throw std::runtime_error { "Unidentified increment operator: " + expression.unaryOperator.value };
    }
}

void SemanticAnalysisVisitor::visit(TypeCast& expression) {
    expression.castExpression->accept(*this);

    expression.setResultHolder(currentScope->newTemp(expression.typeSpecifier.getType(), ""));
}

void SemanticAnalysisVisitor::visit(PointerCast& expression) {
    expression.castExpression->accept(*this);

    expression.setResultHolder(currentScope->newTemp(expression.type.getType(), expression.pointer->getExtendedType()));
}

void SemanticAnalysisVisitor::visit(ArithmeticExpression& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);

    string check = currentScope->typeCheck(expression.leftHandSide->getResultHolder(), expression.rightHandSide->getResultHolder());
    if (check != "ok") {
        error(check, expression.arithmeticOperator.line);
    } else {
        expression.setResultHolder(currentScope->newTemp(expression.getBasicType(), expression.getExtendedType()));
    }
}

void SemanticAnalysisVisitor::visit(ShiftExpression& expression) {
    expression.shiftExpression->accept(*this);
    expression.additionExpression->accept(*this);

    BasicType shiftByType = expression.additionExpression->getBasicType();
    std::string shiftByExtendedType = expression.additionExpression->getExtendedType();
    if (shiftByType == BasicType::INTEGER && shiftByExtendedType == "") {
        expression.setResultHolder(currentScope->newTemp(expression.getBasicType(), expression.getExtendedType()));
    } else {
        error("argument of type int required for shift expression", expression.shiftOperator.line);
    }
}

void SemanticAnalysisVisitor::visit(ComparisonExpression& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);

    std::string check = currentScope->typeCheck(expression.leftHandSide->getResultHolder(), expression.rightHandSide->getResultHolder());
    if (check != "ok") {
        error(check, expression.comparisonOperator.line);
    } else {
        expression.setResultHolder(currentScope->newTemp(BasicType::INTEGER, ""));
        expression.setTruthyLabel(currentScope->newLabel());
        expression.setFalsyLabel(currentScope->newLabel());
    }
}

void SemanticAnalysisVisitor::visit(BitwiseExpression& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);

    string check = currentScope->typeCheck(expression.leftHandSide->getResultHolder(), expression.rightHandSide->getResultHolder());
    if (check != "ok") {
        error(check, expression.bitwiseOperator.line);
    } else {
        expression.setResultHolder(currentScope->newTemp(expression.getBasicType(), expression.getExtendedType()));
    }
}

void SemanticAnalysisVisitor::visit(LogicalAndExpression& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);

    std::string check = currentScope->typeCheck(expression.leftHandSide->getResultHolder(), expression.rightHandSide->getResultHolder());
    if (check != "ok") {
        // FIXME: get line number
        error(check, 0);
    } else {
        expression.setResultHolder(currentScope->newTemp(BasicType::INTEGER, ""));
        expression.setExitLabel(currentScope->newLabel());
    }
}

void SemanticAnalysisVisitor::visit(LogicalOrExpression& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);

    std::string check = currentScope->typeCheck(expression.leftHandSide->getResultHolder(), expression.rightHandSide->getResultHolder());
    if (check != "ok") {
        // FIXME: get line number
        error(check, 0);
    } else {
        expression.setResultHolder(currentScope->newTemp(BasicType::INTEGER, ""));
        expression.setExitLabel(currentScope->newLabel());
    }
}

void SemanticAnalysisVisitor::visit(AssignmentExpression& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);

    if (!expression.isLval()) {
        error("lvalue required on the left side of assignment", expression.assignmentOperator.line);
    } else {
        auto resultHolder = expression.leftHandSide->getResultHolder();
        string check = currentScope->typeCheck(expression.rightHandSide->getResultHolder(), resultHolder);
        if (check != "ok") {
            error(check, expression.assignmentOperator.line);
        } else {
            expression.setResultHolder(resultHolder);
        }
    }
}

void SemanticAnalysisVisitor::visit(ExpressionList& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);
}

void SemanticAnalysisVisitor::visit(JumpStatement& statement) {
    // TODO: not implemented yet
    throw std::runtime_error { "not implemented" };
}

void SemanticAnalysisVisitor::visit(ReturnStatement& statement) {
    statement.returnExpression->accept(*this);
}

void SemanticAnalysisVisitor::visit(IOStatement& statement) {
    statement.expression->accept(*this);
}

void SemanticAnalysisVisitor::visit(IfStatement& statement) {
    statement.testExpression->accept(*this);
    statement.body->accept(*this);

    statement.setFalsyLabel(currentScope->newLabel());
}

void SemanticAnalysisVisitor::visit(IfElseStatement& statement) {
    statement.testExpression->accept(*this);
    statement.truthyBody->accept(*this);
    statement.falsyBody->accept(*this);

    statement.setFalsyLabel(currentScope->newLabel());
    statement.setExitLabel(currentScope->newLabel());
}

void SemanticAnalysisVisitor::visit(LoopStatement& loop) {
    loop.header->accept(*this);
    loop.body->accept(*this);
}

void SemanticAnalysisVisitor::visit(ForLoopHeader& loopHeader) {
    loopHeader.initialization->accept(*this);
    loopHeader.clause->accept(*this);
    loopHeader.increment->accept(*this);

    loopHeader.setLoopEntry(currentScope->newLabel());
    loopHeader.setLoopExit(currentScope->newLabel());
}

void SemanticAnalysisVisitor::visit(WhileLoopHeader& loopHeader) {
    loopHeader.clause->accept(*this);

    loopHeader.setLoopEntry(currentScope->newLabel());
}

void SemanticAnalysisVisitor::visit(Pointer&) {
}

void SemanticAnalysisVisitor::visit(Identifier&) {
}

void SemanticAnalysisVisitor::visit(FunctionDeclaration& declaration) {
    declaration.parameterList->accept(*this);

    int errLine;
    if (0
            != (errLine = currentScope->insert(declaration.getName(), BasicType::FUNCTION, declaration.getType(),
                    declaration.getLineNumber()))) {
        error("symbol `" + declaration.getName() + "` declaration conflicts with previous declaration on line " + std::to_string(errLine),
                declaration.getLineNumber());
    } else {
        auto place = currentScope->lookup(declaration.getName());
        for (auto& parameter : declaredParameters) {
            place->setParam(parameter);
        }
        declaration.setHolder(place);
    }
}

void SemanticAnalysisVisitor::visit(ArrayDeclaration& declaration) {
    declaration.subscriptExpression->accept(*this);
    throw std::runtime_error { "not implemented" };
}

void SemanticAnalysisVisitor::visit(ParameterDeclaration& parameter) {
    auto basicType = parameter.type.getType();
    auto extended_type = parameter.declaration->getType();
    auto name = parameter.declaration->getName();
    if (basicType == BasicType::VOID && extended_type == "") {
        error("error: function argument ‘" + name + "’ declared void", parameter.declaration->getLineNumber());
    } else {
        auto place = new SymbolEntry(name, basicType, extended_type, false, parameter.declaration->getLineNumber());
        place->setParam();
        declaredParameters.push_back(place);
    }
}

void SemanticAnalysisVisitor::visit(FunctionDefinition& function) {
    function.declaration->accept(*this);
    function.body->accept(*this);
}

void SemanticAnalysisVisitor::visit(VariableDeclaration& variableDeclaration) {
    BasicType basicType = variableDeclaration.declaredType.getType();
    for (const auto& declaredVariable : variableDeclaration.declaredVariables->getDeclarations()) {
        size_t lineNumber = declaredVariable->getLineNumber();
        int errLine;
        if (basicType == BasicType::VOID && declaredVariable->getType() == "") {
            error("variable ‘" + declaredVariable->getName() + "’ declared void", lineNumber);
        } else if (0 != (errLine = currentScope->insert(declaredVariable->getName(), basicType, declaredVariable->getType(), lineNumber))) {
            error(
                    "symbol `" + declaredVariable->getName() + "` declaration conflicts with previous declaration on line "
                            + std::to_string(errLine), lineNumber);
        } else {
            declaredVariable->setHolder(currentScope->lookup(declaredVariable->getName()));
        }
    }
}

void SemanticAnalysisVisitor::visit(VariableDefinition& definition) {
    definition.declaration->accept(*this);
    definition.initializerExpression->accept(*this);
}

void SemanticAnalysisVisitor::visit(Block& block) {
    currentScope = currentScope->newScope();
    for (auto parameter : declaredParameters) {
        currentScope->insertParam(parameter->getName(), parameter->getBasicType(), parameter->getExtendedType(), parameter->getLine());
    }
    declaredParameters.clear();
    for (const auto& child : block.getChildren()) {
        child->accept(*this);
    }
    currentScope = currentScope->getOuterScope();
}

void SemanticAnalysisVisitor::visit(ListCarrier& listCarrier) {
    for (const auto& child : listCarrier.getChildren()) {
        child->accept(*this);
    }
}

void SemanticAnalysisVisitor::visit(TranslationUnit& translationUnit) {
    for (const auto& child : translationUnit.getChildren()) {
        child->accept(*this);
    }
}

void SemanticAnalysisVisitor::error(std::string message, size_t lineNumber) {
    containsSemanticErrors = true;
    std::cerr << std::to_string(lineNumber) + ": error: " + message << "\n";
}

bool SemanticAnalysisVisitor::successfulSemanticAnalysis() const {
    return !containsSemanticErrors;
}

std::unique_ptr<SymbolTable> SemanticAnalysisVisitor::getSymbolTable() {
    return std::move(symbolTable);
}

} /* namespace semantic_analyzer */

