#include "SemanticAnalysisVisitor.h"

#include <stddef.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "../ast/ArrayAccess.h"
#include "../ast/ArrayDeclaration.h"
#include "../ast/AssignmentExpressionList.h"
#include "../ast/types/BaseType.h"
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
#include "../ast/Term.h"
#include "../ast/TranslationUnit.h"
#include "../ast/TypeCast.h"
#include "../ast/types/Type.h"
#include "../ast/TypeSpecifier.h"
#include "../ast/UnaryExpression.h"
#include "../ast/VariableDeclaration.h"
#include "../ast/VariableDefinition.h"
#include "../ast/WhileLoopHeader.h"
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
#include "ast/types/Function.h"

class TranslationUnit;

namespace semantic_analyzer {

SemanticAnalysisVisitor::SemanticAnalysisVisitor() :
        symbolTable { new code_generator::SymbolTable { } }, currentScope { symbolTable.get() } {
}

SemanticAnalysisVisitor::~SemanticAnalysisVisitor() {
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

    auto typeInfo = arrayAccess.getLeftOperand()->getTypeInfo();
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

    auto resultHolder = functionCall.getOperand()->getResultHolder();
    auto& arguments = functionCall.getArgumentList()->getExpressions();
    if (arguments.size() != resultHolder->getParamCount()) {
        semanticError("no match for function " + resultHolder->getName(), functionCall.getContext());
    } else {
        std::vector<code_generator::ValueEntry*> declaredArguments = resultHolder->getParams();
        for (size_t i { 0 }; i < arguments.size(); ++i) {
            code_generator::ValueEntry* param = arguments.at(i)->getResultHolder();
            std::string check = currentScope->typeCheck(declaredArguments.at(i), param);
            if ("ok" != check) {
                semanticError(check, functionCall.getContext());
            }
        }

        ast::Type resultType { resultHolder->getType() };
        if (!resultType.isPlainVoid()) {
            resultHolder = currentScope->newTemp(resultType);
        }
        functionCall.setResultHolder(resultHolder);
    }
}

void SemanticAnalysisVisitor::visit(ast::Term& term) {
    if (term.getType() == "id") {
        if (currentScope->hasSymbol(term.getValue())) {
            term.setResultHolder(currentScope->lookup(term.getValue()));
        } else {
            semanticError("symbol `" + term.getValue() + "` is not defined", term.getContext());
        }
    } else {
        term.setResultHolder(currentScope->newTemp(term.getTypeInfo()));
    }
}

void SemanticAnalysisVisitor::visit(ast::PostfixExpression& expression) {
    expression.getOperand()->accept(*this);

    expression.setType(expression.getOperand()->getTypeInfo());
    if (!expression.getOperand()->isLval()) {
        semanticError("lvalue required as increment operand", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::PrefixExpression& expression) {
    expression.getOperand()->accept(*this);

    expression.setType(expression.getOperand()->getTypeInfo());
    if (!expression.getOperand()->isLval()) {
        semanticError("lvalue required as increment operand", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::UnaryExpression& expression) {
    expression.getOperand()->accept(*this);

    switch (expression.getOperator()->getLexeme().at(0)) {
    case '&':
        expression.setResultHolder(currentScope->newTemp(expression.getOperand()->getTypeInfo().getAddressType()));
        break;
    case '*':
        if (expression.getOperand()->getTypeInfo().isPointer()) {
            expression.setResultHolder(currentScope->newTemp(expression.getOperand()->getTypeInfo().getTypePointedTo()));
        } else {
            semanticError("invalid type argument of ‘unary *’ :" + expression.getOperand()->getTypeInfo().toString(), expression.getContext());
        }
        break;
    case '+':
        break;
    case '-':
        expression.setResultHolder(currentScope->newTemp(expression.getOperand()->getTypeInfo()));
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

    expression.setResultHolder(currentScope->newTemp( { expression.getType().getType(), expression.getPointer()->getDereferenceCount() }));
}

void SemanticAnalysisVisitor::visit(ast::ArithmeticExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);
    // FIXME: type conversion
    expression.setType(expression.getLeftOperand()->getTypeInfo());

    std::string check = currentScope->typeCheck(expression.getLeftOperand()->getResultHolder(), expression.getRightOperand()->getResultHolder());
    if (check != "ok") {
        semanticError(check, expression.getContext());
    } else {
        expression.setResultHolder(currentScope->newTemp(expression.getTypeInfo()));
    }
}

void SemanticAnalysisVisitor::visit(ast::ShiftExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    if (expression.getRightOperand()->getTypeInfo().isPlainInteger()) {
        expression.setResultHolder(currentScope->newTemp(expression.getLeftOperand()->getTypeInfo()));
    } else {
        semanticError("argument of type int required for shift expression", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ComparisonExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    std::string check = currentScope->typeCheck(expression.getLeftOperand()->getResultHolder(), expression.getRightOperand()->getResultHolder());
    if (check != "ok") {
        semanticError(check, expression.getContext());
    } else {
        expression.setResultHolder(currentScope->newTemp( { ast::BaseType::newInteger() }));
        expression.setTruthyLabel(currentScope->newLabel());
        expression.setFalsyLabel(currentScope->newLabel());
    }
}

void SemanticAnalysisVisitor::visit(ast::BitwiseExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);
    expression.setType(expression.getLeftOperand()->getTypeInfo());

    std::string check = currentScope->typeCheck(expression.getLeftOperand()->getResultHolder(), expression.getRightOperand()->getResultHolder());
    if (check != "ok") {
        semanticError(check, expression.getContext());
    } else {
        expression.setResultHolder(currentScope->newTemp(expression.getTypeInfo()));
    }
}

void SemanticAnalysisVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    std::string check = currentScope->typeCheck(expression.getLeftOperand()->getResultHolder(), expression.getRightOperand()->getResultHolder());
    if (check != "ok") {
        semanticError(check, expression.getContext());
    } else {
        expression.setResultHolder(currentScope->newTemp( { ast::BaseType::newInteger() }));
        expression.setExitLabel(currentScope->newLabel());
    }
}

void SemanticAnalysisVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    std::string check = currentScope->typeCheck(expression.getLeftOperand()->getResultHolder(), expression.getRightOperand()->getResultHolder());
    if (check != "ok") {
        semanticError(check, expression.getContext());
    } else {
        expression.setResultHolder(currentScope->newTemp( { ast::BaseType::newInteger() }));
        expression.setExitLabel(currentScope->newLabel());
    }
}

void SemanticAnalysisVisitor::visit(ast::AssignmentExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);
    // FIXME: type conversion
    expression.setType(expression.getLeftOperand()->getTypeInfo());

    if (!expression.isLval()) {
        semanticError("lvalue required on the left side of assignment", expression.getContext());
    } else {
        auto resultHolder = expression.getLeftOperand()->getResultHolder();
        std::string check = currentScope->typeCheck(expression.getRightOperand()->getResultHolder(), resultHolder);
        if (check != "ok") {
            semanticError(check, expression.getContext());
        } else {
            expression.setResultHolder(resultHolder);
        }
    }
}

void SemanticAnalysisVisitor::visit(ast::ExpressionList& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);
    expression.setType(expression.getLeftOperand()->getTypeInfo());
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

    std::vector<ast::Type> argumentTypes;
    for (auto& parameterDeclaration : declaration.parameterList->getDeclaredParameters()) {
        argumentTypes.push_back(parameterDeclaration->getTypeInfo());
    }
    std::unique_ptr<ast::BaseType> functionType { new ast::Function { argumentTypes } };

    int errLine;
    if (0
            != (errLine = currentScope->insert(declaration.getName(), { std::move(functionType), declaration.getDereferenceCount() },
                    declaration.getContext().getOffset()))) {
        semanticError("symbol `" + declaration.getName() + "` declaration conflicts with previous declaration on line " + std::to_string(errLine),
                declaration.getContext());
    } else {
        auto place = currentScope->lookup(declaration.getName());
        for (auto& parameter : declaredParameters) { // XXX:? declaration.parameterList->getDeclaredParameters()
            place->addParam(parameter);
        }
        declaration.setHolder(place);
    }
}

void SemanticAnalysisVisitor::visit(ast::ArrayDeclaration& declaration) {
    declaration.subscriptExpression->accept(*this);
    throw std::runtime_error { "not implemented" };
}

void SemanticAnalysisVisitor::visit(ast::ParameterDeclaration& parameter) {
    auto name = parameter.declaration->getName();
    if (parameter.getTypeInfo().isPlainVoid()) {
        semanticError("error: function argument ‘" + name + "’ declared void", parameter.declaration->getContext());
    } else {
        auto paramEntry = new code_generator::ValueEntry(name, parameter.getTypeInfo(), false, parameter.declaration->getContext().getOffset());
        paramEntry->setParam();
        declaredParameters.push_back(paramEntry);
    }
}

void SemanticAnalysisVisitor::visit(ast::FunctionDefinition& function) {
    function.declaration->accept(*this);
    function.body->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::VariableDeclaration& variableDeclaration) {
    for (const auto& declaredVariable : variableDeclaration.declaredVariables->getDeclarations()) {
        size_t lineNumber = declaredVariable->getContext().getOffset();
        int errLine;
        ast::Type declaredType { variableDeclaration.declaredType.getType(), declaredVariable->getDereferenceCount() };
        if (declaredType.isPlainVoid()) {
            semanticError("variable ‘" + declaredVariable->getName() + "’ declared void", declaredVariable->getContext());
        } else if (0 != (errLine = currentScope->insert(declaredVariable->getName(), declaredType, lineNumber))) {
            semanticError("symbol `" + declaredVariable->getName() + "` declaration conflicts with previous declaration on line " + std::to_string(errLine),
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

void SemanticAnalysisVisitor::visit(ast::Block& block) {
    currentScope = currentScope->newScope();
    for (auto parameter : declaredParameters) {
        currentScope->insertParam(parameter->getName(), parameter->getType(), parameter->getLine());
    }
    declaredParameters.clear();
    for (const auto& child : block.getChildren()) {
        child->accept(*this);
    }
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

void SemanticAnalysisVisitor::semanticError(std::string message, const TranslationUnitContext& context) {
    containsSemanticErrors = true;
    std::cerr << context << ": error: " << message << "\n";
}

bool SemanticAnalysisVisitor::successfulSemanticAnalysis() const {
    return !containsSemanticErrors;
}

std::unique_ptr<code_generator::SymbolTable> SemanticAnalysisVisitor::getSymbolTable() {
    return std::move(symbolTable);
}

} /* namespace semantic_analyzer */

