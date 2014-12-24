#include "SemanticAnalysisVisitor.h"

#include <stddef.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "../ast/ArrayAccess.h"
#include "../ast/ArrayDeclaration.h"
#include "../ast/AssignmentExpressionList.h"
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
#include "../code_generator/symbol_table.h"
#include "../scanner/TranslationUnitContext.h"
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

SemanticAnalysisVisitor::SemanticAnalysisVisitor() :
        symbolTable { new code_generator::SymbolTable { } },
        currentScope { symbolTable.get() }
{
}

SemanticAnalysisVisitor::~SemanticAnalysisVisitor() {
    std::cout << "\nsymbol table\n";
    symbolTable->printTable();
    std::cout << "symbol table end\n\n";
}

void SemanticAnalysisVisitor::visit(ast::TypeSpecifier&) {
}

void SemanticAnalysisVisitor::visit(ast::ParameterList& parameterList) {
    for (auto& declaredParameter : parameterList.getDeclaredParameters()) {
        declaredParameter->accept(*this);
    }
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
    arrayAccess.getLeftOperand()->accept(*this);
    arrayAccess.getRightOperand()->accept(*this);

    auto typeInfo = arrayAccess.getLeftOperand()->getType();
    if (typeInfo.isPointer()) {
        auto dereferencedType = typeInfo.getTypePointedTo();
        arrayAccess.setLvalue(currentScope->newTemp(dereferencedType));
        arrayAccess.setResultHolder(currentScope->newTemp(dereferencedType));
    } else {
        semanticError("invalid type for operator[]\n", arrayAccess.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::FunctionCall& functionCall) {
    functionCall.getOperand()->accept(*this);
    functionCall.getArgumentList()->accept(*this);

    // FIXME: try/catch for undefined functions
    auto functionSymbol = currentScope->findFunction(functionCall.getOperand()->getResultHolder()->getName());

    functionCall.setSymbol(functionSymbol);

    auto& arguments = functionCall.getArgumentList()->getExpressions();
    if (arguments.size() == functionSymbol.argumentCount()) {
        std::vector<ast::Type> declaredArgumentTypes = functionSymbol.argumentTypes();
        for (size_t i { 0 }; i < arguments.size(); ++i) {
            const auto& declaredArgumentType = declaredArgumentTypes.at(i);
            const auto& actualArgument = arguments.at(i)->getResultHolder();
            typeCheck(actualArgument->getType(), declaredArgumentType, functionCall.getContext());
        }

        ast::Type returnType { functionSymbol.returnType() };
        if (!returnType.isPlainVoid()) {
            functionCall.setResultHolder(currentScope->newTemp(returnType));
        }
    } else {
        semanticError("no match for function " + functionSymbol.getType().toString(), functionCall.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::Term& term) {
    if (term.getTypeSymbol() == "id") {
        if (currentScope->hasSymbol(term.getValue())) {
            term.setResultHolder(currentScope->lookup(term.getValue()));
        } else {
            semanticError("symbol `" + term.getValue() + "` is not defined", term.getContext());
        }
    } else {
        term.setResultHolder(currentScope->newTemp(term.getType()));
    }
}

void SemanticAnalysisVisitor::visit(ast::PostfixExpression& expression) {
    expression.getOperand()->accept(*this);

    expression.setType(expression.getOperand()->getType());
    if (!expression.getOperand()->isLval()) {
        semanticError("lvalue required as increment operand", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::PrefixExpression& expression) {
    expression.getOperand()->accept(*this);

    expression.setType(expression.getOperand()->getType());
    if (!expression.getOperand()->isLval()) {
        semanticError("lvalue required as increment operand", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::UnaryExpression& expression) {
    expression.getOperand()->accept(*this);

    switch (expression.getOperator()->getLexeme().at(0)) {
    case '&':
        expression.setResultHolder(currentScope->newTemp(expression.getOperand()->getType().getAddressType()));
        break;
    case '*':
        if (expression.getOperand()->getType().isPointer()) {
            expression.setResultHolder(currentScope->newTemp(expression.getOperand()->getType().getTypePointedTo()));
        } else {
            semanticError("invalid type argument of ‘unary *’ :" + expression.getOperand()->getType().toString(),
                    expression.getContext());
        }
        break;
    case '+':
        break;
    case '-':
        expression.setResultHolder(currentScope->newTemp(expression.getOperand()->getType()));
        break;
    case '!':
        expression.setResultHolder(currentScope->newTemp( { ast::BaseType::newInteger() }));
        expression.setTruthyLabel(currentScope->newLabel());
        expression.setFalsyLabel(currentScope->newLabel());
        break;
    default:
        throw std::runtime_error { "Unidentified increment operator: " + expression.getOperator()->getLexeme() };
    }
}

void SemanticAnalysisVisitor::visit(ast::TypeCast& expression) {
    expression.getOperand()->accept(*this);

    expression.setResultHolder(currentScope->newTemp( { expression.getType().getType() }));
}

void SemanticAnalysisVisitor::visit(ast::PointerCast& expression) {
    expression.getOperand()->accept(*this);

    expression.setResultHolder(
            currentScope->newTemp( { expression.getType().getType(), expression.getPointer()->getDereferenceCount() }));
}

void SemanticAnalysisVisitor::visit(ast::ArithmeticExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    typeCheck(
            expression.getLeftOperand()->getType(),
            expression.getRightOperand()->getType(),
            expression.getContext());
    // FIXME: type conversion
    expression.setResultHolder(currentScope->newTemp(expression.getLeftOperand()->getType()));
}

void SemanticAnalysisVisitor::visit(ast::ShiftExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    if (expression.getRightOperand()->getType().isPlainInteger()) {
        expression.setResultHolder(currentScope->newTemp(expression.getLeftOperand()->getType()));
    } else {
        semanticError("argument of type int required for shift expression", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ComparisonExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    typeCheck(
            expression.getLeftOperand()->getType(),
            expression.getRightOperand()->getType(),
            expression.getContext());

    expression.setResultHolder(currentScope->newTemp( { ast::BaseType::newInteger() }));
    expression.setTruthyLabel(currentScope->newLabel());
    expression.setFalsyLabel(currentScope->newLabel());
}

void SemanticAnalysisVisitor::visit(ast::BitwiseExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);
    expression.setType(expression.getLeftOperand()->getType());

    typeCheck(
            expression.getLeftOperand()->getType(),
            expression.getRightOperand()->getType(),
            expression.getContext());

    expression.setResultHolder(currentScope->newTemp(expression.getType()));
}

void SemanticAnalysisVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    typeCheck(
            expression.getLeftOperand()->getType(),
            expression.getRightOperand()->getType(),
            expression.getContext());

    expression.setResultHolder(currentScope->newTemp( { ast::BaseType::newInteger() }));
    expression.setExitLabel(currentScope->newLabel());
}

void SemanticAnalysisVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    typeCheck(
            expression.getLeftOperand()->getType(),
            expression.getRightOperand()->getType(),
            expression.getContext());

    expression.setResultHolder(currentScope->newTemp( { ast::BaseType::newInteger() }));
    expression.setExitLabel(currentScope->newLabel());
}

void SemanticAnalysisVisitor::visit(ast::AssignmentExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    if (expression.isLval()) {
        typeCheck(
                expression.getLeftOperand()->getType(),
                expression.getRightOperand()->getType(),
                expression.getContext());

        expression.setResultHolder(*expression.getLeftOperand()->getResultHolder());
    } else {
        semanticError("lvalue required on the left side of assignment", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ExpressionList& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);
    expression.setType(expression.getLeftOperand()->getType());
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

    statement.setFalsyLabel(currentScope->newLabel());
}

void SemanticAnalysisVisitor::visit(ast::IfElseStatement& statement) {
    statement.testExpression->accept(*this);
    statement.truthyBody->accept(*this);
    statement.falsyBody->accept(*this);

    statement.setFalsyLabel(currentScope->newLabel());
    statement.setExitLabel(currentScope->newLabel());
}

void SemanticAnalysisVisitor::visit(ast::LoopStatement& loop) {
    loop.header->accept(*this);
    loop.body->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::ForLoopHeader& loopHeader) {
    loopHeader.initialization->accept(*this);
    loopHeader.clause->accept(*this);
    loopHeader.increment->accept(*this);

    loopHeader.setLoopEntry(currentScope->newLabel());
    loopHeader.setLoopExit(currentScope->newLabel());
}

void SemanticAnalysisVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    loopHeader.clause->accept(*this);

    loopHeader.setLoopEntry(currentScope->newLabel());
}

void SemanticAnalysisVisitor::visit(ast::Pointer&) {
}

void SemanticAnalysisVisitor::visit(ast::Identifier&) {
}

void SemanticAnalysisVisitor::visit(ast::FunctionDeclaration& declaration) {
    declaration.parameterList->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::ArrayDeclaration& declaration) {
    declaration.subscriptExpression->accept(*this);
    throw std::runtime_error { "not implemented" };
}

void SemanticAnalysisVisitor::visit(ast::ParameterDeclaration& parameter) {
    auto name = parameter.declaration->getName();
    if (parameter.getType().isPlainVoid()) {
        semanticError("error: function argument ‘" + name + "’ declared void", parameter.declaration->getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::VariableDeclaration& variableDeclaration) {
    for (const auto& declaredVariable : variableDeclaration.declaredVariables->getDeclarations()) {
        size_t lineNumber = declaredVariable->getContext().getOffset();
        int errLine;
        ast::Type declaredType { variableDeclaration.declaredType.getType(), declaredVariable->getDereferenceCount() };
        if (declaredType.isPlainVoid()) {
            semanticError("variable ‘" + declaredVariable->getName() + "’ declared void",
                    declaredVariable->getContext());
        } else if (0 != (errLine = currentScope->insert(declaredVariable->getName(), declaredType, lineNumber))) {
            semanticError(
                    "symbol `" + declaredVariable->getName()
                            + "` declaration conflicts with previous declaration on line " + std::to_string(errLine),
                    declaredVariable->getContext());
        } else {
            declaredVariable->setHolder(currentScope->lookup(declaredVariable->getName()));
        }
    }
}

void SemanticAnalysisVisitor::visit(ast::VariableDefinition& definition) {
    definition.declaration->accept(*this);
    definition.initializerExpression->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::FunctionDefinition& function) {
    function.declaration->accept(*this);

    std::vector<ast::Type> argumentTypes;
    for (auto& parameterDeclaration : function.declaration->parameterList->getDeclaredParameters()) {
        argumentTypes.push_back(parameterDeclaration->getType());
    }
    // FIXME: fix the grammar! can only return base types now!
    std::unique_ptr<ast::BaseType> functionType { new ast::Function { function.returnType.getType(), argumentTypes } };

    code_generator::FunctionEntry functionEntry = currentScope->insertFunction(
            function.declaration->getName(),
            { function.returnType.getType(), argumentTypes },
            function.declaration->getContext().getOffset());

    function.setSymbol(functionEntry);
    if (functionEntry.getContext() != function.declaration->getContext().getOffset()) {
        semanticError(
                "function `" + function.declaration->getName() + "` definition conflicts with previous one on line "
                        + std::to_string(functionEntry.getContext()), function.declaration->getContext());
    }

    int errLine;
    if (0 != (errLine = currentScope->insert(function.declaration->getName(),
            { std::move(functionType), function.declaration->getDereferenceCount() },
            function.declaration->getContext().getOffset())))
            {
        semanticError(
                "symbol `" + function.declaration->getName()
                        + "` declaration conflicts with previous declaration on line "
                        + std::to_string(errLine), function.declaration->getContext());
    } else {
        auto place = currentScope->lookup(function.declaration->getName());
        // XXX: might need to move this to definition
        function.declaration->setHolder(place);
    }

    currentScope = currentScope->newScope();
    for (auto& parameter : function.declaration->parameterList->getDeclaredParameters()) {
        currentScope->insertFunctionArgument(parameter->declaration->getName(), parameter->getType(),
                parameter->declaration->getContext().getOffset());
    }
    function.body->accept(*this);
    currentScope = currentScope->getOuterScope();
}

void SemanticAnalysisVisitor::visit(ast::Block& block) {
    currentScope = currentScope->newScope();
    for (const auto& child : block.getChildren()) {
        child->accept(*this);
    }
    block.setSize(currentScope->getTableSize());
    currentScope = currentScope->getOuterScope();
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

void SemanticAnalysisVisitor::typeCheck(const ast::Type& typeFrom, const ast::Type& typeTo,
        const TranslationUnitContext& context)
{
    if (!typeFrom.canConvertTo(typeTo)) {
        semanticError("type mismatch: can't convert " + typeFrom.toString() + " to " + typeTo.toString(), context);
    }
}

void SemanticAnalysisVisitor::semanticError(std::string message, const TranslationUnitContext& context) {
    containsSemanticErrors = true;
    std::cerr << context << ": error: " << message << "\n";
}

bool SemanticAnalysisVisitor::successfulSemanticAnalysis() const {
    return !containsSemanticErrors;
}

} /* namespace semantic_analyzer */

