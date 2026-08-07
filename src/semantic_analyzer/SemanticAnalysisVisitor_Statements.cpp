#include "SemanticAnalysisVisitorInternal.h"

namespace semantic_analyzer {

void SemanticAnalysisVisitor::visit(ast::JumpStatement& statement) {
    if (loopStack.empty()) {
        semanticError("`" + statement.jumpKeyword.type + "` statement not in loop or switch",
                statement.jumpKeyword.context);
        return;
    }
    const auto& loop = loopStack.back();
    if (statement.jumpKeyword.type == "break") {
        statement.setJumpTo(annotations(), *loop.exit);
    } else if (statement.jumpKeyword.type == "continue") {
        if (!loop.cont) {
            semanticError("`continue` statement not in loop", statement.jumpKeyword.context);
            return;
        }
        statement.setJumpTo(annotations(), *loop.cont);
    } else {
        semanticError("unsupported jump statement `" + statement.jumpKeyword.type + "`", statement.jumpKeyword.context);
    }
}

void SemanticAnalysisVisitor::visit(ast::SwitchStatement& statement) {
    statement.expression->accept(*this);
    if (statement.expression->hasResultSymbol(annotations())) {
        rejectFunctionValue(statement.expression->getResultSymbol(annotations())->getType(),
                statement.expression->getContext());
    }

    auto exitLabel = symbolTable.newLabel();
    statement.setExitLabel(annotations(), exitLabel);
    statement.setCaseTemp(annotations(), symbolTable.createTemporarySymbol(type::signedInteger()));

    LabelEntry* continueLabel = nullptr;
    if (!loopStack.empty()) {
        continueLabel = loopStack.back().cont;
    }
    // break → switch exit; continue only if nested in a loop (cont may be null).
    loopStack.push_back({ nullptr, continueLabel, statement.getExitLabel(annotations()) });
    switchStack.push_back(&statement);

    statement.body->accept(*this);

    switchStack.pop_back();
    loopStack.pop_back();
}

void SemanticAnalysisVisitor::visit(ast::CaseLabel& statement) {
    // Always attach a codegen label so the node is well-formed even when illegal.
    statement.setLabel(annotations(), symbolTable.newLabel());

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
    statement.setLabel(annotations(), symbolTable.newLabel());

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
    statement.setLabel(annotations(), label);
    if (namedLabels.find(statement.getLabelName()) != namedLabels.end()) {
        semanticError("duplicate label `" + statement.getLabelName() + "`", statement.name.context);
    } else {
        namedLabels.insert({ statement.getLabelName(), label });
    }
    statement.statement->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::ReturnStatement& statement) {
    statement.returnExpression->accept(*this);
    if (!statement.returnExpression->hasResultSymbol(annotations())) {
        return;
    }
    auto* retExpr = statement.returnExpression.get();
    if (retExpr->holdsAggregateAddress() && currentReturnType && currentReturnType->isRecord()) {
        semanticError("returning dual-type aggregate address is not supported",
                retExpr->getContext());
        return;
    }
    type::Type dest = currentReturnType ? *currentReturnType : type::voidType();
    type::Type retVal = currentReturnType ? assignSourceType(*retExpr, dest, annotations()) : retExpr->getType();
    rejectFunctionValue(retVal, retExpr->getContext());
    if (currentReturnType) {
        typeCheck(retVal, *currentReturnType, retExpr->getContext());
        // Float<->int needs SSE convert before placing the return value in rax/xmm0.
        maybeSetFloatIntConversion(retExpr, *currentReturnType, symbolTable, annotations());
    }
}

void SemanticAnalysisVisitor::visit(ast::VoidReturnStatement& statement) {
}

void SemanticAnalysisVisitor::visit(ast::IfStatement& statement) {
    statement.testExpression->accept(*this);
    if (statement.testExpression->hasResultSymbol(annotations())) {
        rejectFunctionValue(statement.testExpression->getResultSymbol(annotations())->getType(),
                statement.testExpression->getContext());
    }
    statement.body->accept(*this);

    statement.setFalsyLabel(annotations(), symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::IfElseStatement& statement) {
    statement.testExpression->accept(*this);
    if (statement.testExpression->hasResultSymbol(annotations())) {
        rejectFunctionValue(statement.testExpression->getResultSymbol(annotations())->getType(),
                statement.testExpression->getContext());
    }
    statement.truthyBody->accept(*this);
    statement.falsyBody->accept(*this);

    statement.setFalsyLabel(annotations(), symbolTable.newLabel());
    statement.setExitLabel(annotations(), symbolTable.newLabel());
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
        loop.header->setLoopContinue(annotations(), symbolTable.newLabel());
    } else if (loop.header->continueTargetsEntry()) {
        loop.header->setLoopContinue(annotations(), *loop.header->getLoopEntry(annotations()));
    }
    loopStack.push_back({ loop.header->getLoopEntry(annotations()), loop.header->getLoopContinue(annotations()), loop.header->getLoopExit(annotations()) });
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

    loopHeader.setLoopEntry(annotations(), symbolTable.newLabel());
    loopHeader.setLoopExit(annotations(), symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    loopHeader.clause->accept(*this);

    loopHeader.setLoopEntry(annotations(), symbolTable.newLabel());
    loopHeader.setLoopExit(annotations(), symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::DoWhileLoopHeader& loopHeader) {
    loopHeader.clause->accept(*this);

    loopHeader.setLoopEntry(annotations(), symbolTable.newLabel());
    // continue jumps here (re-test), not to the body entry.
    loopHeader.setLoopContinue(annotations(), symbolTable.newLabel());
    loopHeader.setLoopExit(annotations(), symbolTable.newLabel());
}


} // namespace semantic_analyzer
