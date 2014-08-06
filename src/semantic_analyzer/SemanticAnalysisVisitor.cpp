#include "SemanticAnalysisVisitor.h"

#include <stddef.h>
#include <iostream>
#include <string>

#include "../code_generator/symbol_table.h"
#include "BasicType.h"
#include "FunctionDeclaration.h"
#include "FunctionDefinition.h"
#include "TypeSpecifier.h"
#include "VariableDeclaration.h"

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
}

void SemanticAnalysisVisitor::visit(const AssignmentExpressionList& expressions) {
}

void SemanticAnalysisVisitor::visit(const DeclarationList& declarations) {
}

void SemanticAnalysisVisitor::visit(const ArrayAccess& arrayAccess) {
}

void SemanticAnalysisVisitor::visit(const FunctionCall& functionCall) {

}

void SemanticAnalysisVisitor::visit(const NoArgFunctionCall& functionCall) {

}

void SemanticAnalysisVisitor::visit(const Term& term) {
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
}

void SemanticAnalysisVisitor::visit(const ArrayDeclaration& declaration) {
}

void SemanticAnalysisVisitor::visit(const ParameterDeclaration& parameter) {
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
        } else if (0 != (errLine = symbolTable->insert(declaredVariable->getName(), basicType, declaredVariable->getType(), lineNumber))) {
            error(
                    "symbol `" + declaredVariable->getName() + "` declaration conflicts with previous declaration on line "
                            + std::to_string(errLine), lineNumber);
        }
    }
}

void SemanticAnalysisVisitor::visit(const VariableDefinition& definition) {
}

void SemanticAnalysisVisitor::visit(const Block& block) {
    currentScope = currentScope->newScope();
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

