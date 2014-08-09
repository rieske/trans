#include "SemanticAnalysisVisitor.h"

#include <iostream>
#include <stdexcept>

#include "../code_generator/symbol_table.h"
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
#include "Pointer.h"
#include "PointerCast.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "ShiftExpression.h"
#include "Term.h"
#include "TerminalSymbol.h"
#include "TypeCast.h"
#include "TypeSpecifier.h"
#include "VariableDeclaration.h"
#include "VariableDefinition.h"
#include "WhileLoopHeader.h"

namespace semantic_analyzer {

SemanticAnalysisVisitor::SemanticAnalysisVisitor() :
        symbolTable { new SymbolTable { } },
        currentScope { symbolTable.get() } {
}

SemanticAnalysisVisitor::~SemanticAnalysisVisitor() {
    // FIXME: temporary
    std::cout << "\ntable from visitor:\n";
    symbolTable->printTable();
    std::cout << "table end\n\n";
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
        // TODO: arrayAccess.setLval(currentScope->newTemp(basicType, extendedType));
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
    // XXX is the null check required?
    if (!expression.getResultHolder() || expression.postfixExpression->getValue() == "rval") {
        error("lvalue required as increment operand", expression.postfixOperator.line);
    }
}

void SemanticAnalysisVisitor::visit(PrefixExpression& expression) {
    expression.unaryExpression->accept(*this);
    // XXX is the null check required?
    if (!expression.getResultHolder() || expression.unaryExpression->getValue() == "rval") {
        error("lvalue required as increment operand", expression.incrementOperator.line);
    }
}

void SemanticAnalysisVisitor::visit(UnaryExpression& expression) {
    // TODO: possibly break down into subclasses to avoid a large operator based switch
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

    string check = currentScope->typeCheck(expression.leftHandSide->getResultHolder(),
            expression.rightHandSide->getResultHolder());
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

    std::string check = currentScope->typeCheck(expression.leftHandSide->getResultHolder(),
            expression.rightHandSide->getResultHolder());
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

    string check = currentScope->typeCheck(expression.leftHandSide->getResultHolder(),
            expression.rightHandSide->getResultHolder());
    if (check != "ok") {
        error(check, expression.bitwiseOperator.line);
    } else {
        expression.setResultHolder(currentScope->newTemp(expression.getBasicType(), expression.getExtendedType()));
    }
}

void SemanticAnalysisVisitor::visit(LogicalAndExpression& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);

    std::string check = currentScope->typeCheck(expression.leftHandSide->getResultHolder(),
            expression.rightHandSide->getResultHolder());
    if (check != "ok") {
        // FIXME: get line number
        error(check, 0);
    } else {
        expression.setResultHolder(currentScope->newTemp(BasicType::INTEGER, ""));
    }
}

void SemanticAnalysisVisitor::visit(LogicalOrExpression& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);

    std::string check = currentScope->typeCheck(expression.leftHandSide->getResultHolder(),
            expression.rightHandSide->getResultHolder());
    if (check != "ok") {
        // FIXME: get line number
        error(check, 0);
    } else {
        expression.setResultHolder(currentScope->newTemp(BasicType::INTEGER, ""));
    }
}

void SemanticAnalysisVisitor::visit(AssignmentExpression& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);

    auto resultHolder = expression.leftHandSide->getResultHolder();
    if (expression.getValue() != "lval") {
        if (!expression.getLval()) {
            error("lvalue required on the left side of assignment", expression.assignmentOperator.line);
        }
        resultHolder = expression.getLval();
    }
    string check = currentScope->typeCheck(expression.rightHandSide->getResultHolder(), resultHolder);
    if (check != "ok") {
        error(check, expression.assignmentOperator.line);
    } else {
        expression.setResultHolder(resultHolder);
    }
}

void SemanticAnalysisVisitor::visit(ExpressionList& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);
}

void SemanticAnalysisVisitor::visit(JumpStatement& statement) {
    // TODO: not implemented yet
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
    statement.setTruthyLabel(currentScope->newLabel());
}

void SemanticAnalysisVisitor::visit(LoopStatement& loop) {
    loop.header->accept(*this);
    loop.body->accept(*this);

    loop.setLoopEntry(loop.header->getLoopLabel());
    auto loopExit = currentScope->newLabel();
    loop.setLoopExit(loopExit);
    loop.header->setLoopExit(loopExit);
}

void SemanticAnalysisVisitor::visit(ForLoopHeader& loopHeader) {
    loopHeader.initialization->accept(*this);
    loopHeader.clause->accept(*this);
    loopHeader.increment->accept(*this);

    loopHeader.setLoopEntry(currentScope->newLabel());
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
        error(
                "symbol `" + declaration.getName() + "` declaration conflicts with previous declaration on line "
                        + std::to_string(errLine), declaration.getLineNumber());
    } else {
        SymbolEntry *place = currentScope->lookup(declaration.getName());
        for (auto& parameter : declaredParameters) {
            place->setParam(parameter);
        }
    }
}

void SemanticAnalysisVisitor::visit(ArrayDeclaration& declaration) {
    throw std::runtime_error { "not implemented" };
    declaration.subscriptExpression->accept(*this);
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
        } else if (0
                != (errLine = currentScope->insert(declaredVariable->getName(), basicType, declaredVariable->getType(),
                        lineNumber))) {
            error(
                    "symbol `" + declaredVariable->getName()
                            + "` declaration conflicts with previous declaration on line " + std::to_string(errLine),
                    lineNumber);
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
        currentScope->insertParam(parameter->getName(), parameter->getBasicType(), parameter->getExtendedType(),
                parameter->getLine());
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

} /* namespace semantic_analyzer */

