#include "SemanticAnalysisVisitor.h"

#include <stddef.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "../ast/ArrayAccess.h"
#include "../ast/ArrayDeclarator.h"
#include "../ast/AssignmentExpressionList.h"
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
#include "../ast/ReturnStatement.h"
#include "../ast/types/BaseType.h"
#include "../ast/types/Function.h"
#include "../ast/types/Type.h"
#include "../ast/Term.h"
#include "../ast/TranslationUnit.h"
#include "../ast/TypeCast.h"
#include "../ast/TypeSpecifier.h"
#include "../ast/UnaryExpression.h"
#include "../ast/VariableDeclaration.h"
#include "../ast/VariableDefinition.h"
#include "../ast/WhileLoopHeader.h"
#include "../code_generator/FunctionEntry.h"
#include "../code_generator/LabelEntry.h"
#include "translation_unit/Context.h"
#include "ast/ArithmeticExpression.h"
#include "ast/BitwiseExpression.h"
#include "ast/ExpressionList.h"
#include "ast/LogicalAndExpression.h"
#include "ast/LogicalOrExpression.h"
#include "ast/PostfixExpression.h"
#include "ast/PrefixExpression.h"
#include "ast/ShiftExpression.h"

#include "code_generator/FunctionEntry.h"

class TranslationUnit;

namespace semantic_analyzer {

SemanticAnalysisVisitor::SemanticAnalysisVisitor(std::ostream* errorStream) :
        errorStream { errorStream }
{
}

SemanticAnalysisVisitor::~SemanticAnalysisVisitor() {
    std::cout << "\nsymbol table\n";
    symbolTable.printTable();
    std::cout << "symbol table end\n\n";
}

void SemanticAnalysisVisitor::visit(ast::TypeSpecifier&) {
}

void SemanticAnalysisVisitor::visit(ast::AssignmentExpressionList& expressions) {
    for (auto& expression : expressions.getExpressions()) {
        expression->accept(*this);
    }
}

void SemanticAnalysisVisitor::visit(ast::DeclarationList& declarations) {
    for (auto& declaration : declarations.getDeclarations()) {
        declaration->accept(*this);
    }
}

void SemanticAnalysisVisitor::visit(ast::ArrayAccess& arrayAccess) {
    arrayAccess.visitLeftOperand(*this);
    arrayAccess.visitRightOperand(*this);

    auto typeInfo = arrayAccess.leftOperandType();
    if (typeInfo.isPointer()) {
        auto dereferencedType = typeInfo.getTypePointedTo();
        arrayAccess.setLvalue(symbolTable.createTemporarySymbol(dereferencedType));
        arrayAccess.setResultSymbol(symbolTable.createTemporarySymbol(dereferencedType));
    } else {
        semanticError("invalid type for operator[]\n", arrayAccess.context());
    }
}

void SemanticAnalysisVisitor::visit(ast::FunctionCall& functionCall) {
    functionCall.visitOperand(*this);
    functionCall.getArgumentList()->accept(*this);

    // FIXME: try/catch for undefined functions
    auto functionSymbol = symbolTable.findFunction(functionCall.operandSymbol()->getName());

    functionCall.setSymbol(functionSymbol);

    auto& arguments = functionCall.getArgumentList()->getExpressions();
    if (arguments.size() == functionSymbol.argumentCount()) {
        std::vector<ast::Type> declaredArgumentTypes = functionSymbol.argumentTypes();
        for (size_t i { 0 }; i < arguments.size(); ++i) {
            const auto& declaredArgumentType = declaredArgumentTypes.at(i);
            const auto& actualArgument = arguments.at(i)->getResultSymbol();
            typeCheck(actualArgument->getType(), declaredArgumentType, functionCall.context());
        }

        ast::Type returnType { functionSymbol.returnType() };
        if (!returnType.isPlainVoid()) {
            functionCall.setResultSymbol(symbolTable.createTemporarySymbol(returnType));
        }
    } else {
        semanticError("no match for function " + functionSymbol.getType().toString(), functionCall.context());
    }
}

void SemanticAnalysisVisitor::visit(ast::Term& term) {
    if (term.getTypeSymbol() == "id") {
        if (symbolTable.hasSymbol(term.getValue())) {
            term.setResultSymbol(symbolTable.lookup(term.getValue()));
        } else {
            semanticError("symbol `" + term.getValue() + "` is not defined", term.context());
        }
    } else {
        term.setResultSymbol(symbolTable.createTemporarySymbol(term.getType()));
    }
}

void SemanticAnalysisVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);

    expression.setType(expression.operandType());
    if (!expression.isLval()) {
        semanticError("lvalue required as increment operand", expression.context());
    }
}

void SemanticAnalysisVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);

    expression.setType(expression.operandType());
    if (!expression.isLval()) {
        semanticError("lvalue required as increment operand", expression.context());
    }
}

void SemanticAnalysisVisitor::visit(ast::UnaryExpression& expression) {
    expression.visitOperand(*this);

    switch (expression.getOperator()->getLexeme().at(0)) {
    case '&':
        expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.operandType().getAddressType()));
        break;
    case '*':
        if (expression.operandType().isPointer()) {
            expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.operandType().getTypePointedTo()));
        } else {
            semanticError("invalid type argument of ‘unary *’ :" + expression.operandType().toString(),
                    expression.context());
        }
        break;
    case '+':
        break;
    case '-':
        expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.operandType()));
        break;
    case '!':
        expression.setResultSymbol(symbolTable.createTemporarySymbol( { ast::BaseType::newInteger() }));
        expression.setTruthyLabel(symbolTable.newLabel());
        expression.setFalsyLabel(symbolTable.newLabel());
        break;
    default:
        throw std::runtime_error { "Unidentified increment operator: " + expression.getOperator()->getLexeme() };
    }
}

void SemanticAnalysisVisitor::visit(ast::TypeCast& expression) {
    expression.visitOperand(*this);

    expression.setResultSymbol(symbolTable.createTemporarySymbol( { expression.getType().getType() }));
}

void SemanticAnalysisVisitor::visit(ast::PointerCast& expression) {
    expression.visitOperand(*this);

    expression.setResultSymbol(
            symbolTable.createTemporarySymbol( { expression.getType().getType(), expression.getPointer()->getDereferenceCount() }));
}

void SemanticAnalysisVisitor::visit(ast::ArithmeticExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.context());
    // FIXME: type conversion
    expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.leftOperandType()));
}

void SemanticAnalysisVisitor::visit(ast::ShiftExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (expression.rightOperandType().isPlainInteger()) {
        expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.leftOperandType()));
    } else {
        semanticError("argument of type int required for shift expression", expression.context());
    }
}

void SemanticAnalysisVisitor::visit(ast::ComparisonExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.context());

    expression.setResultSymbol(symbolTable.createTemporarySymbol( { ast::BaseType::newInteger() }));
    expression.setTruthyLabel(symbolTable.newLabel());
    expression.setFalsyLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    expression.setType(expression.leftOperandType());

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.context());

    expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.getType()));
}

void SemanticAnalysisVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.context());

    expression.setResultSymbol(symbolTable.createTemporarySymbol( { ast::BaseType::newInteger() }));
    expression.setExitLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.context());

    expression.setResultSymbol(symbolTable.createTemporarySymbol( { ast::BaseType::newInteger() }));
    expression.setExitLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (expression.isLval()) {
        typeCheck(
                expression.leftOperandType(),
                expression.rightOperandType(),
                expression.context());

        expression.setResultSymbol(*expression.leftOperandSymbol());
    } else {
        semanticError("lvalue required on the left side of assignment", expression.context());
    }
}

void SemanticAnalysisVisitor::visit(ast::ExpressionList& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    expression.setType(expression.leftOperandType());
}

void SemanticAnalysisVisitor::visit(ast::Operator&) {
}

void SemanticAnalysisVisitor::visit(ast::JumpStatement& statement) {
    // TODO: not implemented yet
    throw std::runtime_error { "not implemented" };
}

void SemanticAnalysisVisitor::visit(ast::ReturnStatement& statement) {
    statement.returnExpression->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::IOStatement& statement) {
    statement.expression->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::IfStatement& statement) {
    statement.testExpression->accept(*this);
    statement.body->accept(*this);

    statement.setFalsyLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::IfElseStatement& statement) {
    statement.testExpression->accept(*this);
    statement.truthyBody->accept(*this);
    statement.falsyBody->accept(*this);

    statement.setFalsyLabel(symbolTable.newLabel());
    statement.setExitLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::LoopStatement& loop) {
    loop.header->accept(*this);
    loop.body->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::ForLoopHeader& loopHeader) {
    loopHeader.initialization->accept(*this);
    loopHeader.clause->accept(*this);
    loopHeader.increment->accept(*this);

    loopHeader.setLoopEntry(symbolTable.newLabel());
    loopHeader.setLoopExit(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    loopHeader.clause->accept(*this);

    loopHeader.setLoopEntry(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::Pointer&) {
}

void SemanticAnalysisVisitor::visit(ast::Identifier&) {
}

void SemanticAnalysisVisitor::visit(ast::FunctionDeclarator& declarator) {
    declarator.visitFormalArguments(*this);
}

void SemanticAnalysisVisitor::visit(ast::ArrayDeclarator& declaration) {
    declaration.subscriptExpression->accept(*this);
    throw std::runtime_error { "not implemented" };
}

void SemanticAnalysisVisitor::visit(ast::FormalArgument& parameter) {
    if (parameter.getType().isPlainVoid()) {
        semanticError("function argument ‘" + parameter.getName() + "’ declared void", parameter.getDeclarationContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::VariableDeclaration& variableDeclaration) {
    for (const auto& declaredVariable : variableDeclaration.declaredVariables->getDeclarations()) {
        ast::Type declaredType { variableDeclaration.declaredType.getType(), declaredVariable->getDereferenceCount() };
        if (declaredType.isPlainVoid()) {
            semanticError("variable ‘" + declaredVariable->getName() + "’ declared void", declaredVariable->getContext());
        } else if (!symbolTable.insertSymbol(declaredVariable->getName(), declaredType, declaredVariable->getContext())) {
            semanticError(
                    "symbol `" + declaredVariable->getName() +
                            "` declaration conflicts with previous declaration on " +
                            to_string(symbolTable.lookup(declaredVariable->getName()).getContext()), declaredVariable->getContext());
        } else {
            declaredVariable->setHolder(symbolTable.lookup(declaredVariable->getName()));
        }
    }
}

void SemanticAnalysisVisitor::visit(ast::VariableDefinition& definition) {
    definition.declaration->accept(*this);
    definition.initializerExpression->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::FunctionDefinition& function) {
    function.visitDeclarator(*this);

    std::vector<ast::Type> argumentTypes;
    for (auto& parameterDeclaration : function.getFormalArguments()) {
        argumentTypes.push_back(parameterDeclaration->getType());
    }
    // FIXME: fix the grammar! functions can only return base types now!
    code_generator::FunctionEntry functionEntry = symbolTable.insertFunction(
            function.getName(),
            { function.returnType.getType(), argumentTypes },
            function.getDeclarationContext());

    function.setSymbol(functionEntry);
    if (functionEntry.getContext() != function.getDeclarationContext()) {
        semanticError(
                "function `" + function.getName() + "` definition conflicts with previous one on "
                        + to_string(functionEntry.getContext()), function.getDeclarationContext());
    }

    symbolTable.startScope();
    for (auto& parameter : function.getFormalArguments()) {
        symbolTable.insertFunctionArgument(parameter->getName(), parameter->getType(),
                parameter->getDeclarationContext());
    }
    function.body->accept(*this);
    symbolTable.endScope();
}

void SemanticAnalysisVisitor::visit(ast::Block& block) {
    symbolTable.startScope();
    for (const auto& child : block.getChildren()) {
        child->accept(*this);
    }
    block.setSymbols(symbolTable.getCurrentScopeSymbols());
    block.setArguments(symbolTable.getCurrentScopeArguments());
    symbolTable.endScope();
}

void SemanticAnalysisVisitor::visit(ast::ListCarrier& listCarrier) {
    for (const auto& child : listCarrier.getChildren()) {
        child->accept(*this);
    }
}

void SemanticAnalysisVisitor::visit(ast::TranslationUnit& translationUnit) {
    for (const auto& child : translationUnit.getChildren()) {
        child->accept(*this);
    }
}

void SemanticAnalysisVisitor::typeCheck(const ast::Type& typeFrom, const ast::Type& typeTo, const translation_unit::Context& context) {
    if (!typeFrom.canConvertTo(typeTo)) {
        semanticError("type mismatch: can't convert " + typeFrom.toString() + " to " + typeTo.toString(), context);
    }
}

void SemanticAnalysisVisitor::semanticError(std::string message, const translation_unit::Context& context) {
    containsSemanticErrors = true;
    *errorStream << context << ": error: " << message << "\n";
}

bool SemanticAnalysisVisitor::successfulSemanticAnalysis() const {
    return !containsSemanticErrors;
}

} /* namespace semantic_analyzer */

