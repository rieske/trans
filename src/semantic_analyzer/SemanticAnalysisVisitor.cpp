#include "SemanticAnalysisVisitor.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>

#include "translation_unit/Context.h"
#include "types/Type.h"
#include "util/Logger.h"
#include "util/LogManager.h"

namespace semantic_analyzer {

static const translation_unit::Context EXTERNAL_CONTEXT {"external", 0};

static Logger& err = LogManager::getErrorLogger();

// Locals are stored as `$s<scopeId><name>` in the symbol table; strip that prefix for diagnostics
// and for looking up file-scope functions under their source names.
static std::string unscopedSymbolName(const std::string& name) {
    if (name.size() > 2 && name[0] == '$' && name[1] == 's') {
        std::size_t i = 2;
        while (i < name.size() && std::isdigit(static_cast<unsigned char>(name[i]))) {
            ++i;
        }
        if (i > 2 && i < name.size()) {
            return name.substr(i);
        }
    }
    return name;
}

SemanticAnalysisVisitor::SemanticAnalysisVisitor() {
    type::Type functionType = type::function(type::signedInteger());
    symbolTable.insertFunction("printf", functionType.getFunction(), EXTERNAL_CONTEXT);
    symbolTable.insertFunction("scanf", functionType.getFunction(), EXTERNAL_CONTEXT);
}

SemanticAnalysisVisitor::~SemanticAnalysisVisitor() {
}

void SemanticAnalysisVisitor::printSymbolTable() const {
    symbolTable.printTable();
}

void SemanticAnalysisVisitor::visit(ast::DeclarationSpecifiers& declarationSpecifiers) {
    // FIXME: this would look so much better
    /*for (std::string error : declarationSpecifiers.getSemanticErrors()) {
     semanticError(error, globalContext);
     }*/
    if (declarationSpecifiers.getStorageSpecifiers().size() > 1) {
        semanticError("multiple storage classes in declaration specifiers",
                declarationSpecifiers.getStorageSpecifiers().at(1).getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::Declaration& declaration) {
    declaration.visitChildren(*this);

    auto baseType = declaration.getDeclarationSpecifiers().getTypeSpecifiers().at(0).getType();
    for (const auto& declarator : declaration.getDeclarators()) {
        auto type = declarator->getFundamentalType(baseType);
        if (type.isVoid()) {
            semanticError("variable `" + declarator->getName() + "` declared void", declarator->getContext());
        } else if (symbolTable.isAtFileScope() && symbolTable.hasFunction(declarator->getName())) {
            semanticError("symbol `" + declarator->getName() + "` declaration conflicts with function of the same name",
                    declarator->getContext());
        } else if (symbolTable.insertSymbol(declarator->getName(), type, declarator->getContext())) {
            declarator->setHolder(symbolTable.lookup(declarator->getName()));
            // TODO: type check initializers
            if (declarator->hasInitializer() && symbolTable.isAtFileScope()) {
                long initValue = 0;
                if (declarator->getInitializer()->evaluateConstant(initValue)) {
                    symbolTable.setGlobalInitializer(declarator->getName(), initValue);
                } else {
                    semanticError("global initializer is not a constant expression", declarator->getContext());
                }
            }
        } else {
            semanticError(
                    "symbol `" + declarator->getName() +
                            "` declaration conflicts with previous declaration on " +
                            to_string(symbolTable.lookup(declarator->getName()).getContext()),
                    declarator->getContext());
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

    // Operand failed to resolve (e.g. undeclared identifier) — error already reported.
    if (!arrayAccess.hasLeftOperandSymbol() || !arrayAccess.hasRightOperandSymbol()) {
        return;
    }

    auto type = arrayAccess.leftOperandType();
    if (type.isPointer()) {
        arrayAccess.setLvalue(symbolTable.createTemporarySymbol(type.dereference()));
        arrayAccess.setResultSymbol(symbolTable.createTemporarySymbol(type.dereference()));
    } else {
        semanticError("invalid type for operator[]\n", arrayAccess.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::FunctionCall& functionCall) {
    functionCall.visitOperand(*this);
    functionCall.visitArguments(*this);

    // Operand failed to resolve (e.g. undeclared identifier) — error already reported.
    if (!functionCall.hasOperandSymbol()) {
        return;
    }

    // ValueEntry names for locals are scope-prefixed (e.g. `$s1a`); functions are stored under
    // the source identifier. A mangled name is always a local (or temp), never a function entry —
    // do not look up the demangled form, or a local would incorrectly call a same-named function.
    const auto symbolName = functionCall.operandSymbol()->getName();
    const auto displayName = unscopedSymbolName(symbolName);
    if (symbolName != displayName || !symbolTable.hasFunction(displayName)) {
        semanticError("called object `" + displayName + "` is not a function", functionCall.getContext());
        return;
    }

    auto functionSymbol = symbolTable.findFunction(displayName);

    functionCall.setSymbol(functionSymbol);

    auto& arguments = functionCall.getArgumentList();
    if (arguments.size() == functionSymbol.argumentCount()) {
        auto declaredArguments = functionSymbol.arguments();
        for (std::size_t i { 0 }; i < arguments.size(); ++i) {
            if (!arguments.at(i)->hasResultSymbol()) {
                return;
            }
            const auto& declaredArgument = declaredArguments.at(i);
            const auto& actualArgument = arguments.at(i)->getResultSymbol();
            typeCheck(actualArgument->getType(), declaredArgument, functionCall.getContext());
        }

        auto returnType = functionSymbol.returnType();
        if (!returnType.isVoid()) {
            functionCall.setResultSymbol(symbolTable.createTemporarySymbol(returnType));
        }
    } else if (functionSymbol.getContext() == EXTERNAL_CONTEXT) {
    // FIXME: using EXTERNAL_CONTEXT as a workaround for printf/scanf external functions until varargs are properly implemented
        auto returnType = functionSymbol.returnType();
        if (!returnType.isVoid()) {
            functionCall.setResultSymbol(symbolTable.createTemporarySymbol(returnType));
        }
    } else {
        semanticError("no match for function " + functionSymbol.getType().to_string(), functionCall.getContext());
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

void SemanticAnalysisVisitor::visit(ast::StringLiteralExpression& stringLiteral) {
    std::string constantSymbol = symbolTable.newConstant(stringLiteral.getValue());
    stringLiteral.setConstantSymbol(constantSymbol);
    stringLiteral.setResultSymbol(symbolTable.createTemporarySymbol(stringLiteral.getType()));
}

void SemanticAnalysisVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);
    if (!expression.hasOperandSymbol()) {
        return;
    }

    expression.setType(expression.operandType());
    auto operandSymbol = *expression.operandSymbol();
    expression.setResultSymbol(operandSymbol);

    auto preOperationSymbolName = operandSymbol.getName() + "_pre";
    symbolTable.insertSymbol(preOperationSymbolName, operandSymbol.getType(), operandSymbol.getContext());
    expression.setPreOperationSymbol(symbolTable.lookup(preOperationSymbolName));

    if (!expression.isLval()) {
        semanticError("lvalue required as increment operand", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);
    if (!expression.hasOperandSymbol()) {
        return;
    }

    expression.setType(expression.operandType());
    expression.setResultSymbol(*expression.operandSymbol());

    if (!expression.isLval()) {
        semanticError("lvalue required as increment operand", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::UnaryExpression& expression) {
    expression.visitOperand(*this);
    if (!expression.hasOperandSymbol()) {
        return;
    }

    switch (expression.getOperator()->getLexeme().front()) {
    case '&':
        expression.setResultSymbol(symbolTable.createTemporarySymbol(type::pointer(expression.operandType())));
        break;
    case '*':
        if (expression.operandType().isPointer()) {
            expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.operandType().dereference()));
            expression.setLvalueSymbol(symbolTable.createTemporarySymbol(expression.operandType()));
        } else {
            semanticError("invalid type argument of ‘unary *’ :" + expression.operandType().to_string(), expression.getContext());
        }
        break;
    case '+':
        expression.setResultSymbol(*expression.operandSymbol());
        break;
    case '-':
        expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.operandType()));
        break;
    case '!':
        expression.setResultSymbol(symbolTable.createTemporarySymbol(type::signedInteger()));
        expression.setTruthyLabel(symbolTable.newLabel());
        expression.setFalsyLabel(symbolTable.newLabel());
        break;
    default:
        throw std::runtime_error { "Unidentified increment operator: " + expression.getOperator()->getLexeme() };
    }
}

void SemanticAnalysisVisitor::visit(ast::TypeCast& expression) {
    expression.visitOperand(*this);
    if (!expression.hasOperandSymbol()) {
        return;
    }

    expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.getType().getType()));
}

void SemanticAnalysisVisitor::visit(ast::ArithmeticExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

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
    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    if (expression.rightOperandType().isPrimitive() && !expression.rightOperandType().getPrimitive().isFloating()) {
        expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.leftOperandType()));
    } else {
        semanticError("argument of type int required for shift expression", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::ComparisonExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());

    expression.setResultSymbol(symbolTable.createTemporarySymbol(type::signedInteger()));
    expression.setTruthyLabel(symbolTable.newLabel());
    expression.setFalsyLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }
    expression.setType(expression.leftOperandType());

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());

    expression.setResultSymbol(
            symbolTable.createTemporarySymbol(expression.getType()));
}

void SemanticAnalysisVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());

    expression.setResultSymbol(symbolTable.createTemporarySymbol(type::signedInteger()));
    expression.setExitLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());

    expression.setResultSymbol(symbolTable.createTemporarySymbol(type::signedInteger()));
    expression.setExitLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

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
    if (!expression.hasRightOperandSymbol()) {
        return;
    }
    // Comma operator: value and type of the right operand
    expression.setType(expression.rightOperandType());
    expression.setResultSymbol(*expression.rightOperandSymbol());
}

void SemanticAnalysisVisitor::visit(ast::Operator&) {
}

void SemanticAnalysisVisitor::visit(ast::JumpStatement& statement) {
    throw std::runtime_error { "not implemented" };
}

void SemanticAnalysisVisitor::visit(ast::ReturnStatement& statement) {
    statement.returnExpression->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::VoidReturnStatement& statement) {
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
    if (loopHeader.initialization) {
        loopHeader.initialization->accept(*this);
    }
    if (loopHeader.clause) {
        loopHeader.clause->accept(*this);
    }
    if (loopHeader.increment) {
        loopHeader.increment->accept(*this);
    }

    loopHeader.setLoopEntry(symbolTable.newLabel());
    loopHeader.setLoopExit(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    loopHeader.clause->accept(*this);

    loopHeader.setLoopEntry(symbolTable.newLabel());
    loopHeader.setLoopExit(symbolTable.newLabel());
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

    argumentNames.clear();
    std::vector<type::Type> arguments;
    for (auto& argumentDeclaration : declarator.getFormalArguments()) {
        arguments.push_back(argumentDeclaration.getType());
        argumentNames.push_back(argumentDeclaration.getName());
    }

    // FIXME: return type is not known at this point!
    type::Type functionType = type::function(type::signedInteger(), arguments);
    if (symbolTable.hasGlobalVariable(declarator.getName())) {
        semanticError("function `" + declarator.getName() + "` conflicts with global variable of the same name",
                declarator.getContext());
        return;
    }
    FunctionEntry functionEntry = symbolTable.insertFunction(
            declarator.getName(),
            functionType.getFunction(),
            declarator.getContext());

    if (functionEntry.getContext() != declarator.getContext()) {
        semanticError("function `" + declarator.getName() + "` definition conflicts with previous one on "
                + to_string(functionEntry.getContext()), declarator.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::FormalArgument& argument) {
    argument.visitSpecifiers(*this);
    argument.visitDeclarator(*this);
    if (argument.getType().isVoid()) {
        semanticError("function argument ‘" + argument.getName() + "’ declared void", argument.getDeclarationContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::FunctionDefinition& function) {
    function.visitReturnType(*this);
    function.visitDeclarator(*this);

    if (!symbolTable.hasFunction(function.getName())) {
        return;
    }
    function.setSymbol(symbolTable.findFunction(function.getName()));
    symbolTable.startFunction(function.getName(), argumentNames);
    // Parameters and outermost body declarations share one scope (C); do not enterBlockScope.
    function.visitBodyChildren(*this);
    function.setArguments(symbolTable.getCurrentScopeArguments());
    function.setLocalVariables(symbolTable.getCurrentScopeSymbols());
    symbolTable.endFunction();
}

void SemanticAnalysisVisitor::visit(ast::Block& block) {
    symbolTable.enterBlockScope();
    block.visitChildren(*this);
    symbolTable.exitBlockScope();
}

void SemanticAnalysisVisitor::typeCheck(const type::Type& typeFrom, const type::Type& typeTo,
        const translation_unit::Context& context)
{
    if (!typeTo.canAssignFrom(typeFrom)) {
        semanticError("type mismatch: can't convert " + typeFrom.to_string() + " to " + typeTo.to_string(), context);
    }
}

void SemanticAnalysisVisitor::semanticError(std::string message, const translation_unit::Context& context) {
    containsSemanticErrors = true;
    err << context << ": error: " << message << "\n";
}

bool SemanticAnalysisVisitor::successfulSemanticAnalysis() const {
    return !containsSemanticErrors;
}

std::map<std::string, std::string> SemanticAnalysisVisitor::getConstants() const {
    return symbolTable.getConstants();
}

std::vector<ValueEntry> SemanticAnalysisVisitor::getGlobalVariables() const {
    return symbolTable.getGlobalVariables();
}

} // namespace semantic_analyzer

