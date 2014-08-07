#include "SemanticAnalysisVisitor.h"

#include <iostream>
#include <stdexcept>

#include "../code_generator/symbol_table.h"
#include "ArrayDeclaration.h"
#include "BasicType.h"
#include "FunctionDeclaration.h"
#include "FunctionDefinition.h"
#include "ParameterDeclaration.h"
#include "TypeSpecifier.h"
#include "VariableDeclaration.h"
#include "VariableDefinition.h"

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

void SemanticAnalysisVisitor::visit(const TypeSpecifier& typeSpecifier) {
}

void SemanticAnalysisVisitor::visit(const ParameterList& parameterList) {
    for (auto& declaredParameter : parameterList.getDeclaredParameters()) {
        declaredParameter->accept(*this);
    }
}

void SemanticAnalysisVisitor::visit(const AssignmentExpressionList& expressions) {
    for (auto& expression : expressions.getExpressions()) {
        expression->accept(*this);
    }
}

void SemanticAnalysisVisitor::visit(const DeclarationList& declarations) {
    for (auto& declaration : declarations.getDeclarations()) {
        declaration->accept(*this);
    }
}

void SemanticAnalysisVisitor::visit(const ArrayAccess& arrayAccess) {
}

void SemanticAnalysisVisitor::visit(const FunctionCall& functionCall) {

}

void SemanticAnalysisVisitor::visit(const NoArgFunctionCall& functionCall) {

}

void SemanticAnalysisVisitor::visit(const Term& term) {
    /*if (term.term.type == "id") {
     if ( NULL != (place = currentScope->lookup(term.term.value))) {
     value = "lval";
     basicType = place->getBasicType();
     extended_type = place->getExtendedType();
     } else {
     error("symbol " + term.term.value + " is not defined", term.term.line);
     }
     } else if (term.term.type == "int_const") {
     value = "rval";
     basicType = BasicType::INTEGER;
     place = currentScope->newTemp(basicType, "");
     } else if (term.term.type == "float_const") {
     value = "rval";
     basicType = BasicType::FLOAT;
     place = currentScope->newTemp(basicType, "");
     } else if (term.term.type == "literal") {
     value = "rval";
     basicType = BasicType::CHARACTER;
     place = currentScope->newTemp(basicType, "");
     } else if (term.term.type == "string") {
     value = term.value;
     basicType = BasicType::CHARACTER;
     extended_type = "a";
     place = currentScope->newTemp(basicType, extended_type);
     } else {
     error("bad term literal: " + term.term.value, term.term.line);
     }*/
}

void SemanticAnalysisVisitor::visit(const PostfixExpression& expression) {
}

void SemanticAnalysisVisitor::visit(const PrefixExpression& expression) {
}

void SemanticAnalysisVisitor::visit(const UnaryExpression& expression) {
}

void SemanticAnalysisVisitor::visit(const TypeCast& expression) {
}

void SemanticAnalysisVisitor::visit(const PointerCast& expression) {
}

void SemanticAnalysisVisitor::visit(const ArithmeticExpression& expression) {
}

void SemanticAnalysisVisitor::visit(const ShiftExpression& expression) {
}

void SemanticAnalysisVisitor::visit(const ComparisonExpression& expression) {
}

void SemanticAnalysisVisitor::visit(const BitwiseExpression& expression) {
}

void SemanticAnalysisVisitor::visit(const LogicalAndExpression& expression) {
}

void SemanticAnalysisVisitor::visit(const LogicalOrExpression& expression) {
}

void SemanticAnalysisVisitor::visit(const AssignmentExpression& expression) {
}

void SemanticAnalysisVisitor::visit(const ExpressionList& expression) {
}

void SemanticAnalysisVisitor::visit(const JumpStatement& statement) {
}

void SemanticAnalysisVisitor::visit(const ReturnStatement& statement) {
}

void SemanticAnalysisVisitor::visit(const IOStatement& statement) {
}

void SemanticAnalysisVisitor::visit(const IfStatement& statement) {
}

void SemanticAnalysisVisitor::visit(const IfElseStatement& statement) {
}

void SemanticAnalysisVisitor::visit(const LoopStatement& statement) {
}

void SemanticAnalysisVisitor::visit(const ForLoopHeader& loopHeader) {
}

void SemanticAnalysisVisitor::visit(const WhileLoopHeader& loopHeader) {
}

void SemanticAnalysisVisitor::visit(const Pointer& pointer) {
}

void SemanticAnalysisVisitor::visit(const Identifier& identifier) {
}

void SemanticAnalysisVisitor::visit(const FunctionDeclaration& declaration) {
    declaration.parameterList->accept(*this);

    int errLine;
    if (0
            != (errLine = currentScope->insert(declaration.getName(), BasicType::FUNCTION, declaration.getType(),
                    declaration.getLineNumber()))) {
        error("symbol `" + declaration.getName() + "` declaration conflicts with previous declaration on line " + std::to_string(errLine),
                declaration.getLineNumber());
    } else {
        SymbolEntry *place = currentScope->lookup(declaration.getName());
        for (auto& parameter : declaredParameters) {
            place->setParam(parameter);
        }
    }
}

void SemanticAnalysisVisitor::visit(const ArrayDeclaration& declaration) {
    throw std::runtime_error { "not implemented" };
    declaration.subscriptExpression->accept(*this);
}

void SemanticAnalysisVisitor::visit(const ParameterDeclaration& parameter) {
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

void SemanticAnalysisVisitor::visit(const FunctionDefinition& function) {
    function.declaration->accept(*this);
    function.body->accept(*this);
}

void SemanticAnalysisVisitor::visit(const VariableDeclaration& variableDeclaration) {
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
        }
    }
}

void SemanticAnalysisVisitor::visit(const VariableDefinition& definition) {
    definition.declaration->accept(*this);
    definition.initializerExpression->accept(*this);
}

void SemanticAnalysisVisitor::visit(const Block& block) {
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

void SemanticAnalysisVisitor::visit(const ListCarrier& listCarrier) {
    for (const auto& child : listCarrier.getChildren()) {
        child->accept(*this);
    }
}

void SemanticAnalysisVisitor::visit(const TranslationUnit& translationUnit) {
    for (const auto& child : translationUnit.getChildren()) {
        child->accept(*this);
    }
}

void SemanticAnalysisVisitor::error(std::string message, size_t lineNumber) {
    containsSemanticErrors = true;
    std::cerr << std::to_string(lineNumber) + ": error: " + message << "\n";
}

} /* namespace semantic_analyzer */

