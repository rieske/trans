#include "SemanticAnalysisVisitorInternal.h"

namespace semantic_analyzer {




void SemanticAnalysisVisitor::visit(ast::JumpStatement& statement) {
    if (loopStack.empty()) {
        semanticError("`" + statement.jumpKeyword.type + "` statement not in loop or switch",
                statement.jumpKeyword.context);
        return;
    }
    const auto& labels = loopStack.back();
    if (statement.isBreak()) {
        annotations().setLabel(&statement, symbols::LabelSlot::Target, *labels.breakLabel);
    } else {
        if (!labels.continueLabel) {
            semanticError("`continue` statement not in loop", statement.jumpKeyword.context);
            return;
        }
        annotations().setLabel(&statement, symbols::LabelSlot::Target, *labels.continueLabel);
    }
}

void SemanticAnalysisVisitor::visit(ast::ReturnStatement& statement) {
    // Return value context: visit + array-to-pointer decay (C 6.3.2.1).
    analyzeAsRvalue(statement.returnExpression.get());
    auto* expr = statement.returnExpression.get();
    // C 6.8.6.4: same assign gate + maybeSetConversion as call args (float/int, widen).
    // ntohl uint32 returned as off_t must zero-extend (nth_packed_object_offset).
    if (currentFunctionReturnType && expr && expr->hasResultSymbol(annotations())) {
        if (!checkProductAssign(*currentFunctionReturnType, expr->valueType(annotations()),
                expr->getContext(), expr)) {
            return;
        }
        maybeSetConversion(expr, *currentFunctionReturnType, symbolTable, annotations());
    }
}

void SemanticAnalysisVisitor::visit(ast::VoidReturnStatement& statement) {
}

void SemanticAnalysisVisitor::visit(ast::IfStatement& statement) {
    statement.testExpression->accept(*this);
    statement.body->accept(*this);

    annotations().setLabel(&statement, symbols::LabelSlot::Falsy, symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::IfElseStatement& statement) {
    statement.testExpression->accept(*this);
    statement.truthyBody->accept(*this);
    statement.falsyBody->accept(*this);

    annotations().setLabel(&statement, symbols::LabelSlot::Falsy, symbolTable.newLabel());
    annotations().setLabel(&statement, symbols::LabelSlot::Exit, symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::LoopStatement& loop) {
    // C99 for (decl; ...) introduces a scope covering the whole statement.
    auto* forHeader = dynamic_cast<ast::ForLoopHeader*>(loop.header.get());
    const bool declScope = forHeader && forHeader->isDeclarationInit();
    if (declScope) {
        symbolTable.enterBlockScope();
    }
    loop.header->accept(*this);
    loopStack.push_back({ annotations().label(loop.header.get(), symbols::LabelSlot::LoopExit), annotations().label(loop.header.get(), symbols::LabelSlot::LoopContinue) });
    loop.body->accept(*this);
    loopStack.pop_back();
    if (declScope) {
        symbolTable.exitBlockScope();
    }
}

void SemanticAnalysisVisitor::visit(ast::SwitchStatement& statement) {
    statement.expression->accept(*this);

    auto exitLabel = symbolTable.newLabel();
    annotations().setLabel(&statement, symbols::LabelSlot::Exit, exitLabel);
    annotations().setValue(&statement, symbols::ValueSlot::CaseTemp, symbolTable.createTemporarySymbol(type::signedInteger()));

    symbols::LabelEntry* continueLabel = nullptr;
    if (!loopStack.empty()) {
        continueLabel = loopStack.back().continueLabel;
    }
    loopStack.push_back({ annotations().label(&statement, symbols::LabelSlot::Exit), continueLabel });
    switchStack.push_back(&statement);

    statement.body->accept(*this);

    switchStack.pop_back();
    loopStack.pop_back();
}

void SemanticAnalysisVisitor::visit(ast::CaseLabel& statement) {
    // Always attach a codegen label so the node is well-formed even when illegal.
    annotations().setLabel(&statement, symbols::LabelSlot::Primary, symbolTable.newLabel());

    if (switchStack.empty()) {
        semanticError("case label not within a switch statement", statement.caseExpression->getContext());
        statement.statement->accept(*this);
        return;
    }

    statement.caseExpression->accept(*this);
    long value = 0;
    if (!statement.caseExpression->foldToHostLong(value)) {
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
    annotations().setLabel(&statement, symbols::LabelSlot::Primary, symbolTable.newLabel());

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
    annotations().setLabel(&statement, symbols::LabelSlot::Primary, label);
    if (namedLabels.find(statement.getLabelName()) != namedLabels.end()) {
        semanticError("duplicate label `" + statement.getLabelName() + "`", statement.name.context);
    } else {
        namedLabels.insert({ statement.getLabelName(), label });
    }
    statement.statement->accept(*this);
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

    annotations().setLabel(&loopHeader, symbols::LabelSlot::LoopEntry, symbolTable.newLabel());
    annotations().setLabel(&loopHeader, symbols::LabelSlot::LoopContinue, symbolTable.newLabel());
    annotations().setLabel(&loopHeader, symbols::LabelSlot::LoopExit, symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    loopHeader.clause->accept(*this);

    annotations().setLabel(&loopHeader, symbols::LabelSlot::LoopEntry, symbolTable.newLabel());
    annotations().setLabel(&loopHeader, symbols::LabelSlot::LoopContinue, symbolTable.newLabel());
    annotations().setLabel(&loopHeader, symbols::LabelSlot::LoopExit, symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::DoWhileLoopHeader& loopHeader) {
    loopHeader.clause->accept(*this);

    annotations().setLabel(&loopHeader, symbols::LabelSlot::LoopEntry, symbolTable.newLabel());
    annotations().setLabel(&loopHeader, symbols::LabelSlot::LoopContinue, symbolTable.newLabel());
    annotations().setLabel(&loopHeader, symbols::LabelSlot::LoopExit, symbolTable.newLabel());
}

} // namespace semantic_analyzer
