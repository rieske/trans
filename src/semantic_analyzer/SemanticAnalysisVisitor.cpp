#include "SemanticAnalysisVisitor.h"

#include <iostream>
#include <stdexcept>

#include "../code_generator/symbol_table.h"
#include "ArithmeticExpression.h"
#include "ArrayAccess.h"
#include "ArrayDeclaration.h"
#include "BasicType.h"
#include "FunctionCall.h"
#include "FunctionDeclaration.h"
#include "FunctionDefinition.h"
#include "ParameterDeclaration.h"
#include "Pointer.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "Term.h"
#include "TerminalSymbol.h"
#include "TypeSpecifier.h"
#include "VariableDeclaration.h"
#include "VariableDefinition.h"

//#include "TypeSpecifier.h"
//#include "VariableDeclaration.h"
//#include "VariableDefinition.h"

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

void SemanticAnalysisVisitor::visit(TypeSpecifier& typeSpecifier) {
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
    auto resultPlace = functionCall.callExpression->getPlace();
    auto& arguments = functionCall.argumentList->getExpressions();
    if (arguments.size() != resultPlace->getParamCount()) {
        // FIXME: line number
        error("no match for function " + resultPlace->getName(), 0);
    } else {
        vector<SymbolEntry *> declaredArguments = resultPlace->getParams();
        for (size_t i { 0 }; i < arguments.size(); ++i) {
            SymbolEntry *param = arguments.at(i)->getPlace();
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
}

void SemanticAnalysisVisitor::visit(TypeCast& expression) {
}

void SemanticAnalysisVisitor::visit(PointerCast& expression) {
}

void SemanticAnalysisVisitor::visit(ArithmeticExpression& expression) {
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);

    auto leftOperand = expression.leftHandSide->getResultHolder();
    auto rightOperand = expression.rightHandSide->getResultHolder();
    string check = currentScope->typeCheck(leftOperand, rightOperand);
    if (check != "ok") {
        error(check, expression.arithmeticOperator.line);
    } else {
        expression.setResultHolder(currentScope->newTemp(expression.getBasicType(), expression.getExtendedType()));
    }
}

void SemanticAnalysisVisitor::visit(ShiftExpression& expression) {
}

void SemanticAnalysisVisitor::visit(ComparisonExpression& expression) {
}

void SemanticAnalysisVisitor::visit(BitwiseExpression& expression) {
}

void SemanticAnalysisVisitor::visit(LogicalAndExpression& expression) {
}

void SemanticAnalysisVisitor::visit(LogicalOrExpression& expression) {
}

void SemanticAnalysisVisitor::visit(AssignmentExpression& expression) {
}

void SemanticAnalysisVisitor::visit(ExpressionList& expression) {
}

void SemanticAnalysisVisitor::visit(JumpStatement& statement) {
}

void SemanticAnalysisVisitor::visit(ReturnStatement& statement) {
    statement.returnExpression->accept(*this);
}

void SemanticAnalysisVisitor::visit(IOStatement& statement) {
}

void SemanticAnalysisVisitor::visit(IfStatement& statement) {
}

void SemanticAnalysisVisitor::visit(IfElseStatement& statement) {
}

void SemanticAnalysisVisitor::visit(LoopStatement& statement) {
}

void SemanticAnalysisVisitor::visit(ForLoopHeader& loopHeader) {
}

void SemanticAnalysisVisitor::visit(WhileLoopHeader& loopHeader) {
}

void SemanticAnalysisVisitor::visit(Pointer& pointer) {
}

void SemanticAnalysisVisitor::visit(Identifier& identifier) {
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

