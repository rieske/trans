#include "SemanticAnalysisVisitor.h"

#include <algorithm>
#include <cctype>
#include <limits>
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
    // Reject function designators as values (e.g. printf("%d", main)) — codegen has no address.
    for (auto& argument : arguments) {
        if (argument->hasResultSymbol()) {
            rejectFunctionValue(argument->getResultSymbol()->getType(), functionCall.getContext());
        }
    }

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
    rejectFunctionValue(expression.operandType(), expression.getContext());

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
    rejectFunctionValue(expression.operandType(), expression.getContext());

    expression.setType(expression.operandType());
    expression.setResultSymbol(*expression.operandSymbol());

    if (!expression.isLval()) {
        semanticError("lvalue required as increment operand", expression.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::UnaryExpression& expression) {
    const auto& lexeme = expression.getOperator()->getLexeme();
    if (lexeme == "sizeof") {
        expression.visitOperand(*this);
        if (!expression.hasOperandSymbol()) {
            expression.setResultSymbol(symbolTable.createTemporarySymbol(type::signedInteger()));
            return;
        }
        expression.setSizeofValue(expression.operandType().getSize());
        expression.setResultSymbol(symbolTable.createTemporarySymbol(type::signedInteger()));
        return;
    }

    expression.visitOperand(*this);
    if (!expression.hasOperandSymbol()) {
        return;
    }
    rejectFunctionValue(expression.operandType(), expression.getContext());

    switch (lexeme.front()) {
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
    case '~':
        expression.setResultSymbol(symbolTable.createTemporarySymbol(expression.operandType()));
        break;
    case '!':
        expression.setResultSymbol(symbolTable.createTemporarySymbol(type::signedInteger()));
        expression.setTruthyLabel(symbolTable.newLabel());
        expression.setFalsyLabel(symbolTable.newLabel());
        break;
    default:
        throw std::runtime_error { "Unidentified unary operator: " + expression.getOperator()->getLexeme() };
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
    rejectFunctionValue(expression.leftOperandType(), expression.getContext());
    rejectFunctionValue(expression.rightOperandType(), expression.getContext());

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
    rejectFunctionValue(expression.leftOperandType(), expression.getContext());
    rejectFunctionValue(expression.rightOperandType(), expression.getContext());

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
    rejectFunctionValue(expression.leftOperandType(), expression.getContext());
    rejectFunctionValue(expression.rightOperandType(), expression.getContext());

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
    rejectFunctionValue(expression.leftOperandType(), expression.getContext());
    rejectFunctionValue(expression.rightOperandType(), expression.getContext());
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
    rejectFunctionValue(expression.leftOperandType(), expression.getContext());
    rejectFunctionValue(expression.rightOperandType(), expression.getContext());

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
    rejectFunctionValue(expression.leftOperandType(), expression.getContext());
    rejectFunctionValue(expression.rightOperandType(), expression.getContext());

    typeCheck(
            expression.leftOperandType(),
            expression.rightOperandType(),
            expression.getContext());

    expression.setResultSymbol(symbolTable.createTemporarySymbol(type::signedInteger()));
    expression.setExitLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::ConditionalExpression& expression) {
    expression.visitCondition(*this);
    expression.visitTrueExpression(*this);
    expression.visitFalseExpression(*this);

    if (!expression.getCondition()->hasResultSymbol()
            || !expression.getTrueExpression()->hasResultSymbol()
            || !expression.getFalseExpression()->hasResultSymbol()) {
        return;
    }

    rejectFunctionValue(expression.conditionSymbol()->getType(), expression.getContext());
    rejectFunctionValue(expression.trueSymbol()->getType(), expression.getContext());
    rejectFunctionValue(expression.falseSymbol()->getType(), expression.getContext());

    typeCheck(
            expression.trueSymbol()->getType(),
            expression.falseSymbol()->getType(),
            expression.getContext());

    // Result type follows the true arm after typeCheck (same policy as other binary ops).
    const type::Type resultType = expression.trueSymbol()->getType();
    expression.setType(resultType);
    expression.setResultSymbol(symbolTable.createTemporarySymbol(resultType));
    expression.setFalsyLabel(symbolTable.newLabel());
    expression.setExitLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    if (!expression.hasLeftOperandSymbol() || !expression.hasRightOperandSymbol()) {
        return;
    }

    if (expression.isLval()) {
        rejectFunctionValue(expression.leftOperandType(), expression.getContext());
        rejectFunctionValue(expression.rightOperandType(), expression.getContext());
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
    if (loopStack.empty()) {
        semanticError("`" + statement.jumpKeyword.type + "` statement not in loop or switch",
                statement.jumpKeyword.context);
        return;
    }
    const auto& loop = loopStack.back();
    if (statement.jumpKeyword.type == "break") {
        statement.setJumpTo(*loop.exit);
    } else if (statement.jumpKeyword.type == "continue") {
        if (!loop.cont) {
            semanticError("`continue` statement not in loop", statement.jumpKeyword.context);
            return;
        }
        statement.setJumpTo(*loop.cont);
    } else {
        semanticError("unsupported jump statement `" + statement.jumpKeyword.type + "`", statement.jumpKeyword.context);
    }
}

void SemanticAnalysisVisitor::visit(ast::SwitchStatement& statement) {
    statement.expression->accept(*this);
    if (statement.expression->hasResultSymbol()) {
        rejectFunctionValue(statement.expression->getResultSymbol()->getType(),
                statement.expression->getContext());
    }

    auto exitLabel = symbolTable.newLabel();
    statement.setExitLabel(exitLabel);
    statement.setCaseTemp(symbolTable.createTemporarySymbol(type::signedInteger()));

    LabelEntry* continueLabel = nullptr;
    if (!loopStack.empty()) {
        continueLabel = loopStack.back().cont;
    }
    // break → switch exit; continue only if nested in a loop (cont may be null).
    loopStack.push_back({ nullptr, continueLabel, statement.getExitLabel() });
    switchStack.push_back(&statement);

    statement.body->accept(*this);

    switchStack.pop_back();
    loopStack.pop_back();
}

void SemanticAnalysisVisitor::visit(ast::CaseLabel& statement) {
    // Always attach a codegen label so the node is well-formed even when illegal.
    statement.setLabel(symbolTable.newLabel());

    if (switchStack.empty()) {
        semanticError("case label not within a switch statement", statement.caseExpression->getContext());
        statement.statement->accept(*this);
        return;
    }

    statement.caseExpression->accept(*this);
    long value = 0;
    if (!statement.caseExpression->evaluateConstant(value)) {
        semanticError("case label is not a constant expression", statement.caseExpression->getContext());
        statement.statement->accept(*this);
        return;
    }
    statement.setCaseValue(value);
    for (const auto* existing : switchStack.back()->getCases()) {
        if (existing->getCaseValue() == value) {
            semanticError("duplicate case value", statement.caseExpression->getContext());
            statement.statement->accept(*this);
            return;
        }
    }
    switchStack.back()->addCase(&statement);

    statement.statement->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::DefaultLabel& statement) {
    // Always attach a label for codegen, even when the label is illegal / duplicate.
    statement.setLabel(symbolTable.newLabel());

    if (switchStack.empty()) {
        semanticError("default label not within a switch statement", statement.defaultKeyword.context);
        statement.statement->accept(*this);
        return;
    }

    if (switchStack.back()->getDefaultLabel()) {
        // Keep the first default for codegen; ignore subsequent ones as targets.
        semanticError("multiple default labels in switch", statement.defaultKeyword.context);
    } else {
        switchStack.back()->setDefaultLabel(&statement);
    }

    statement.statement->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::GotoStatement& statement) {
    pendingGotos.push_back(&statement);
}

void SemanticAnalysisVisitor::visit(ast::LabeledStatement& statement) {
    // Always attach a codegen label so the statement node is well-formed even when
    // the name is a duplicate (goto targets keep the first definition only).
    auto label = symbolTable.newLabel();
    statement.setLabel(label);
    if (namedLabels.find(statement.getLabelName()) != namedLabels.end()) {
        semanticError("duplicate label `" + statement.getLabelName() + "`", statement.name.context);
    } else {
        namedLabels.insert({ statement.getLabelName(), label });
    }
    statement.statement->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::ReturnStatement& statement) {
    statement.returnExpression->accept(*this);
    if (statement.returnExpression->hasResultSymbol()) {
        rejectFunctionValue(statement.returnExpression->getResultSymbol()->getType(),
                statement.returnExpression->getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::VoidReturnStatement& statement) {
}

void SemanticAnalysisVisitor::visit(ast::IfStatement& statement) {
    statement.testExpression->accept(*this);
    if (statement.testExpression->hasResultSymbol()) {
        rejectFunctionValue(statement.testExpression->getResultSymbol()->getType(),
                statement.testExpression->getContext());
    }
    statement.body->accept(*this);

    statement.setFalsyLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::IfElseStatement& statement) {
    statement.testExpression->accept(*this);
    if (statement.testExpression->hasResultSymbol()) {
        rejectFunctionValue(statement.testExpression->getResultSymbol()->getType(),
                statement.testExpression->getContext());
    }
    statement.truthyBody->accept(*this);
    statement.falsyBody->accept(*this);

    statement.setFalsyLabel(symbolTable.newLabel());
    statement.setExitLabel(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::LoopStatement& loop) {
    const bool declScope = loop.header->opensBlockScope();
    if (declScope) {
        symbolTable.enterBlockScope();
    }
    loop.header->accept(*this);
    // for-with-increment: continue before increment. while: continue → entry.
    // do-while: header preassigns continue (before the test); leave it alone.
    if (loop.header->increment) {
        loop.header->setLoopContinue(symbolTable.newLabel());
    } else if (loop.header->continueTargetsEntry()) {
        loop.header->setLoopContinue(*loop.header->getLoopEntry());
    }
    loopStack.push_back({ loop.header->getLoopEntry(), loop.header->getLoopContinue(), loop.header->getLoopExit() });
    loop.body->accept(*this);
    loopStack.pop_back();
    if (declScope) {
        symbolTable.exitBlockScope();
    }
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

void SemanticAnalysisVisitor::visit(ast::DoWhileLoopHeader& loopHeader) {
    loopHeader.clause->accept(*this);

    loopHeader.setLoopEntry(symbolTable.newLabel());
    // continue jumps here (re-test), not to the body entry.
    loopHeader.setLoopContinue(symbolTable.newLabel());
    loopHeader.setLoopExit(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::Pointer&) {
}

void SemanticAnalysisVisitor::visit(ast::Identifier&) {
}

void SemanticAnalysisVisitor::visit(ast::ArrayDeclarator& declaration) {
    declaration.visitBaseDeclarator(*this);
    if (declaration.subscriptExpression) {
        declaration.subscriptExpression->accept(*this);
        long length = 0;
        if (!declaration.subscriptExpression->evaluateConstant(length) || length < 0) {
            semanticError("array size is not a non-negative constant expression",
                    declaration.getContext());
            declaration.setArraySize(0);
        } else if (length > static_cast<long>(std::numeric_limits<int>::max())) {
            semanticError("array size is too large", declaration.getContext());
            declaration.setArraySize(0);
        } else {
            declaration.setArraySize(length);
        }
    } else {
        // Incomplete array T a[] — treat as zero-length for now.
        declaration.setArraySize(0);
    }
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
    namedLabels.clear();
    pendingGotos.clear();
    // Parameters and outermost body declarations share one scope (C); do not enterBlockScope.
    function.visitBodyChildren(*this);
    for (auto* gotoStmt : pendingGotos) {
        auto it = namedLabels.find(gotoStmt->getLabelName());
        if (it == namedLabels.end()) {
            semanticError("label `" + gotoStmt->getLabelName() + "` used but not defined",
                    gotoStmt->label.context);
        } else {
            gotoStmt->setTarget(it->second);
        }
    }
    namedLabels.clear();
    pendingGotos.clear();
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

void SemanticAnalysisVisitor::rejectFunctionValue(const type::Type& type, const translation_unit::Context& context) {
    // Pointer-to-function types are stored as function types with indirection > 0, so they
    // also report isFunction(). Only bare function designators (no pointer) are rejected.
    if (type.isFunction() && !type.isPointer()) {
        semanticError("function designator used as a value is not supported", context);
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

