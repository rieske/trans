#include "SemanticAnalysisVisitorInternal.h"

namespace semantic_analyzer {



void SemanticAnalysisVisitor::visit(ast::JumpStatement& statement) {
    if (loopStack.empty()) {
        semanticError(statement.jumpKeyword.type + " statement not within a loop or switch", statement.jumpKeyword.context);
        return;
    }
    const auto& labels = loopStack.back();
    if (statement.isBreak()) {
        statement.setTarget(*labels.breakLabel);
    } else {
        if (!labels.continueLabel) {
            semanticError("continue statement not within a loop", statement.jumpKeyword.context);
            return;
        }
        statement.setTarget(*labels.continueLabel);
    }
}

void SemanticAnalysisVisitor::visit(ast::ReturnStatement& statement) {
    statement.returnExpression->accept(*this);
    // Array-to-pointer decay on return (C 6.3.2.1): return buf; yields &buf[0].
    auto* expr = statement.returnExpression.get();
    decayArrayInPlace(expr, symbolTable);
    // Implicit conversion of return value to function return type (C 6.8.6.4).
    // Float<->int needs SSE convert (cvttsd2si / cvtsi2sd); without it, bare
    // `return score * 100 / 60000.0` leaves double bits / xmm0 for an int return
    // (git similarity_index / rename detection).
    // Integer width change also needs a convert temp: a 32-bit stack store of
    // ntohl's result reloaded as 64-bit off_t keeps garbage in the high half
    // (git nth_packed_object_offset / index v1: "offset beyond end of packfile").
    // Keep expression result as the source type so codegen writes the source there;
    // conversion target is a separate temp of the function return type.
    if (currentFunctionReturnType) {
        // Returns: float↔int and integral width changes (ntohl → off_t).
        maybeSetNumericConversion(expr, *currentFunctionReturnType, symbolTable,
                /*includeIntegralWidthChange=*/true);
    }
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
    // C99 for (decl; ...) introduces a scope covering the whole statement.
    auto* forHeader = dynamic_cast<ast::ForLoopHeader*>(loop.header.get());
    const bool declScope = forHeader && forHeader->declarationInit;
    if (declScope) {
        symbolTable.enterBlockScope();
    }
    loop.header->accept(*this);
    loopStack.push_back({ loop.header->getLoopExit(), loop.header->getLoopContinue() });
    loop.body->accept(*this);
    loopStack.pop_back();
    if (declScope) {
        symbolTable.exitBlockScope();
    }
}

void SemanticAnalysisVisitor::visit(ast::SwitchStatement& statement) {
    statement.expression->accept(*this);

    auto exitLabel = symbolTable.newLabel();
    statement.setExitLabel(exitLabel);
    statement.setCaseTemp(symbolTable.createTemporarySymbol(type::signedInteger()));

    LabelEntry* continueLabel = nullptr;
    if (!loopStack.empty()) {
        continueLabel = loopStack.back().continueLabel;
    }
    loopStack.push_back({ statement.getExitLabel(), continueLabel });
    switchStack.push_back(&statement);

    statement.body->accept(*this);

    switchStack.pop_back();
    loopStack.pop_back();
}

void SemanticAnalysisVisitor::visit(ast::CaseLabel& statement) {
    if (switchStack.empty()) {
        semanticError("case label not within a switch statement", statement.caseExpression->getContext());
        statement.statement->accept(*this);
        return;
    }

    statement.caseExpression->accept(*this);
    long value = 0;
    if (!statement.caseExpression->evaluateConstant(value)) {
        semanticError("case label is not a constant expression", statement.caseExpression->getContext());
    }
    statement.setCaseValue(value);
    statement.setLabel(symbolTable.newLabel());
    switchStack.back()->addCase(&statement);

    statement.statement->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::DefaultLabel& statement) {
    if (switchStack.empty()) {
        semanticError("default label not within a switch statement", externalContext());
        statement.statement->accept(*this);
        return;
    }

    if (switchStack.back()->getDefaultLabel()) {
        semanticError("multiple default labels in switch", externalContext());
    }
    statement.setLabel(symbolTable.newLabel());
    switchStack.back()->setDefaultLabel(&statement);

    statement.statement->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::GotoStatement& statement) {
    pendingGotos.push_back(&statement);
}

void SemanticAnalysisVisitor::visit(ast::LabeledStatement& statement) {
    if (namedLabels.find(statement.getLabelName()) != namedLabels.end()) {
        semanticError("duplicate label `" + statement.getLabelName() + "`", statement.name.context);
    } else {
        auto label = symbolTable.newLabel();
        namedLabels.insert({ statement.getLabelName(), label });
        statement.setLabel(label);
    }
    statement.statement->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::ForLoopHeader& loopHeader) {
    if (loopHeader.declarationInit) {
        loopHeader.declarationInit->accept(*this);
    }
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
    loopHeader.setLoopContinue(symbolTable.newLabel());
    loopHeader.setLoopExit(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    loopHeader.clause->accept(*this);

    loopHeader.setLoopEntry(symbolTable.newLabel());
    loopHeader.setLoopContinue(symbolTable.newLabel());
    loopHeader.setLoopExit(symbolTable.newLabel());
}

void SemanticAnalysisVisitor::visit(ast::DoWhileLoopHeader& loopHeader) {
    loopHeader.clause->accept(*this);

    loopHeader.setLoopEntry(symbolTable.newLabel());
    loopHeader.setLoopContinue(symbolTable.newLabel());
    loopHeader.setLoopExit(symbolTable.newLabel());
}

} // namespace semantic_analyzer
