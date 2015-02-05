#include "SemanticAnalysisVisitor.h"

#include <stddef.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "../ast/ArrayAccess.h"
#include "../ast/ArrayDeclarator.h"
#include "../ast/Block.h"
#include "../ast/ComparisonExpression.h"
#include "../ast/ForLoopHeader.h"
#include "../ast/FunctionCall.h"
#include "../ast/FunctionDeclarator.h"
#include "../ast/FunctionDefinition.h"
#include "../ast/Identifier.h"
#include "../ast/IfElseStatement.h"
#include "../ast/IfStatement.h"
#include "../ast/IOStatement.h"
#include "../ast/JumpStatement.h"
#include "../ast/LogicalExpression.h"
#include "../ast/LoopStatement.h"
#include "../ast/Operator.h"
#include "../ast/FormalArgument.h"
#include "../ast/Pointer.h"
#include "../ast/ReturnStatement.h"
#include "../ast/types/BaseType.h"
#include "../ast/types/Function.h"
#include "../ast/types/Type.h"
#include "../ast/IdentifierExpression.h"
#include "../ast/ConstantExpression.h"
#include "../ast/TypeCast.h"
#include "../ast/TypeSpecifier.h"
#include "../ast/UnaryExpression.h"
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
#include "ast/AssignmentExpression.h"
#include "ast/types/Function.h"
#include "ast/Declarator.h"
#include "ast/InitializedDeclarator.h"
#include "ast/Declaration.h"

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

void SemanticAnalysisVisitor::visit(ast::DeclarationSpecifiers& declarationSpecifiers) {
    // FIXME: this would look so much better
    /*for (std::string error : declarationSpecifiers.getSemanticErrors()) {
     semanticError(error, globalContext);
     }*/
    if (declarationSpecifiers.getStorageSpecifiers().size() > 1) {
        semanticError("multiple storage classes in declaration specifiers", declarationSpecifiers.getStorageSpecifiers().at(1).getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::Declaration& declaration) {
    declaration.visitChildren(*this);

    auto baseType = declaration.getDeclarationSpecifiers().getTypeSpecifiers().at(0).getType();
    for (const auto& declarator : declaration.getDeclarators()) {
        ast::Type type { baseType->clone(), declarator->getDereferenceCount() };
        if (type.isPlainVoid()) {
            semanticError("variable ‘" + declarator->getName() + "’ declared void", declarator->getContext());
        } else if (!symbolTable.insertSymbol(declarator->getName(), type, declarator->getContext())) {
            semanticError(
                    "symbol `" + declarator->getName() +
                            "` declaration conflicts with previous declaration on " +
                            to_string(symbolTable.lookup(declarator->getName()).getContext()), declarator->getContext());
        } else {
            declarator->setHolder(symbolTable.lookup(declarator->getName()));
            // TODO: type check initializers
        }
    }
}

void SemanticAnalysisVisitor::visit(ast::Declarator& declarator) {
    declarator.visitChildren(*this);
}

void SemanticAnalysisVisitor::visit(ast::InitializedDeclarator& declarator) {
    declarator.visitChildren(*this);
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
        semanticError("invalid type for operator[]\n", arrayAccess.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::FunctionCall& functionCall) {
    functionCall.visitOperand(*this);
    functionCall.visitArguments(*this);

    // FIXME: try/catch for undefined functions
    auto functionSymbol = symbolTable.findFunction(functionCall.operandSymbol()->getName());

    functionCall.setSymbol(functionSymbol);

    auto& arguments = functionCall.getArgumentList();
    if (arguments.size() == functionSymbol.argumentCount()) {
        auto declaredArguments = functionSymbol.arguments();
        for (size_t i { 0 }; i < arguments.size(); ++i) {
            const auto& declaredArgument = declaredArguments.at(i);
            const auto& actualArgument = arguments.at(i)->getResultSymbol();
            typeCheck(actualArgument->getType(), declaredArgument.second, functionCall.getContext());
        }

        ast::Type returnType { functionSymbol.returnType() };
        if (!returnType.isPlainVoid()) {
            functionCall.setResultSymbol(symbolTable.createTemporarySymbol(returnType));
        }
    } else {
        semanticError("no match for function " + functionSymbol.getType().toString(), functionCall.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::IdentifierExpression& identifier) {
    if (symbolTable.hasSymbol(identifier.getIdentifier())) {
        identifier.setResultSymbol(symbolTable.lookup(identifier.getIdentifier()));
    } else {
        semanticError("symbol `" + identifier.getIdentifier() + "` is not defined", identifier.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ConstantExpression& constant) {
    constant.setResultSymbol(symbolTable.createTemporarySymbol(constant.getType()));
}

void SemanticAnalysisVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);

    expression.setType(expression.operandType());
    if (!expression.isLval()) {
        semanticError("lvalue required as increment operand", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);

    expression.setType(expression.operandType());
    if (!expression.isLval()) {
        semanticError("lvalue required as increment operand", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::UnaryExpression& expression) {
    expression.visitOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '&':
        expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.operandType().getAddressType()));
        break;
    case '*':
        if (expression.operandType().isPointer()) {
            expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.operandType().getTypePointedTo()));
        } else {
            semanticError("invalid type argument of ‘unary *’ :" + expression.operandType().toString(),
                    expression.getContext());
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

void SemanticAnalysisVisitor::visit(ast::ArithmeticExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());
    // FIXME: type conversion
    expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.leftOperandType()));
}

void SemanticAnalysisVisitor::visit(ast::ShiftExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    if (expression.rightOperandType().isPlainInteger()) {
        expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.leftOperandType()));
    } else {
        semanticError("argument of type int required for shift expression", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ComparisonExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());

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
            expression.getContext());

    expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.getType()));
}

void SemanticAnalysisVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());

    expression.setResultSymbol(symbolTable.createTemporarySymbol( { ast::BaseType::newInteger() }));
    expression.setExitLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());

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
                expression.getContext());

        expression.setResultSymbol(*expression.leftOperandSymbol());
    } else {
        semanticError("lvalue required on the left side of assignment", expression.getContext());
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

void SemanticAnalysisVisitor::visit(ast::ArrayDeclarator& declaration) {
    declaration.subscriptExpression->accept(*this);
    throw std::runtime_error { "not implemented" };
}

void SemanticAnalysisVisitor::visit(ast::FunctionDeclarator& declarator) {
    declarator.visitFormalArguments(*this);

    std::vector<std::pair<std::string, ast::Type>> arguments;
    for (auto& argumentDeclaration : declarator.getFormalArguments()) {
        arguments.push_back(std::make_pair(argumentDeclaration.getName(), argumentDeclaration.getType()));
    }

    code_generator::FunctionEntry functionEntry = symbolTable.insertFunction(
            declarator.getName(),
            { { ast::BaseType::newInteger() }, arguments },
            declarator.getContext());

    if (functionEntry.getContext() != declarator.getContext()) {
        semanticError("function `" + declarator.getName() + "` definition conflicts with previous one on "
                + to_string(functionEntry.getContext()), declarator.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::FormalArgument& argument) {
    argument.visitSpecifiers(*this);
    argument.visitDeclarator(*this);
    if (argument.getType().isPlainVoid()) {
        semanticError("function argument ‘" + argument.getName() + "’ declared void", argument.getDeclarationContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::FunctionDefinition& function) {
    function.visitReturnType(*this);
    function.visitDeclarator(*this);

    function.setSymbol(symbolTable.findFunction(function.getName()));
    symbolTable.startFunction(function.getName());
    function.visitBody(*this);
    symbolTable.endFunction();
}

void SemanticAnalysisVisitor::visit(ast::Block& block) {
    symbolTable.startScope();
    block.visitChildren(*this);
    block.setSymbols(symbolTable.getCurrentScopeSymbols());
    block.setArguments(symbolTable.getCurrentScopeArguments());
    symbolTable.endScope();
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

