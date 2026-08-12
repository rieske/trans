#include "CodeGeneratingVisitor.h"

#include <stdexcept>

#include "Instruction.h"

namespace codegen {

void CodeGeneratingVisitor::visit(ast::JumpStatement& statement) {
    if (!statement.getJumpTo(store_)) {
        throw std::runtime_error { "JumpStatement has no target label" };
    }
    emit(ir::jump(statement.getJumpTo(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::SwitchStatement& statement) {
    statement.expression->accept(*this);

    auto switchResult = statement.expression->getResultSymbol(store_)->getName();
    auto caseTemp = statement.getCaseTemp(store_)->getName();

    for (auto* caseLabel : statement.getCases()) {
        emit(ir::assignConstant(
                std::to_string(caseLabel->getCaseValue()), caseTemp));
        emit(ir::valueCompare(switchResult, caseTemp));
        emit(ir::jump(caseLabel->getLabel(store_)->getName(), JumpCondition::IF_EQUAL));
    }

    if (statement.getDefaultLabel()) {
        emit(ir::jump(statement.getDefaultLabel()->getLabel(store_)->getName()));
    } else {
        emit(ir::jump(statement.getExitLabel(store_)->getName()));
    }

    statement.body->accept(*this);
    emit(ir::label(statement.getExitLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::CaseLabel& statement) {
    emit(ir::label(statement.getLabel(store_)->getName()));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::DefaultLabel& statement) {
    emit(ir::label(statement.getLabel(store_)->getName()));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::GotoStatement& statement) {
    if (!statement.getTarget(store_)) {
        throw std::runtime_error { "GotoStatement has no target label" };
    }
    emit(ir::jump(statement.getTarget(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::LabeledStatement& statement) {
    if (!statement.getLabel(store_)) {
        throw std::runtime_error { "LabeledStatement has no label" };
    }
    emit(ir::label(statement.getLabel(store_)->getName()));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::ReturnStatement& statement) {
    statement.returnExpression->accept(*this);
    emit(ir::ret(convertedResultName(*statement.returnExpression)));
}

void CodeGeneratingVisitor::visit(ast::VoidReturnStatement& statement) {
    (void)statement;
    emit(ir::voidReturn());
}

void CodeGeneratingVisitor::visit(ast::IfStatement& statement) {
    statement.testExpression->accept(*this);

    emit(ir::zeroCompare(statement.testExpression->getResultSymbol(store_)->getName()));
    emit(ir::jump(statement.getFalsyLabel(store_)->getName(), JumpCondition::IF_EQUAL));

    statement.body->accept(*this);

    emit(ir::label(statement.getFalsyLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::IfElseStatement& statement) {
    statement.testExpression->accept(*this);

    emit(ir::zeroCompare(statement.testExpression->getResultSymbol(store_)->getName()));
    emit(ir::jump(statement.getFalsyLabel(store_)->getName(), JumpCondition::IF_EQUAL));

    statement.truthyBody->accept(*this);
    emit(ir::jump(statement.getExitLabel(store_)->getName()));
    emit(ir::label(statement.getFalsyLabel(store_)->getName()));

    statement.falsyBody->accept(*this);
    emit(ir::label(statement.getExitLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::LoopStatement& loop) {
    if (loop.header->bodyBeforeTest()) {
        // do { body } while (cond); - header visit emits the trailing test + branch.
        emit(ir::label(loop.header->getLoopEntry(store_)->getName()));
        loop.body->accept(*this);
        emit(ir::label(loop.header->getLoopContinue(store_)->getName()));
        loop.header->accept(*this);
        emit(ir::label(loop.header->getLoopExit(store_)->getName()));
        return;
    }

    loop.header->accept(*this);
    loop.body->accept(*this);
    // continue target: for-loops place a label before the increment; while reuses entry.
    if (loop.header->getLoopContinue(store_)
            && loop.header->getLoopContinue(store_)->getName() != loop.header->getLoopEntry(store_)->getName()) {
        emit(ir::label(loop.header->getLoopContinue(store_)->getName()));
    }
    if (loop.header->increment) {
        loop.header->increment->accept(*this);
    }

    emit(ir::jump(loop.header->getLoopEntry(store_)->getName()));
    emit(ir::label(loop.header->getLoopExit(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::ForLoopHeader& loopHeader) {
    if (loopHeader.initialization) {
        loopHeader.initialization->accept(*this);
    }

    emit(ir::label(loopHeader.getLoopEntry(store_)->getName()));
    if (loopHeader.clause) {
        loopHeader.clause->accept(*this);
        emit(ir::zeroCompare(loopHeader.clause->getResultSymbol(store_)->getName()));
        emit(ir::jump(loopHeader.getLoopExit(store_)->getName(), JumpCondition::IF_EQUAL));
    }
}

void CodeGeneratingVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    emit(ir::label(loopHeader.getLoopEntry(store_)->getName()));
    loopHeader.clause->accept(*this);
    emit(ir::zeroCompare(loopHeader.clause->getResultSymbol(store_)->getName()));
    emit(ir::jump(loopHeader.getLoopExit(store_)->getName(), JumpCondition::IF_EQUAL));
}

void CodeGeneratingVisitor::visit(ast::DoWhileLoopHeader& loopHeader) {
    // Invoked after the body and continue label (see visit(LoopStatement)).
    loopHeader.clause->accept(*this);
    emit(ir::zeroCompare(loopHeader.clause->getResultSymbol(store_)->getName()));
    emit(ir::jump(loopHeader.getLoopEntry(store_)->getName(), JumpCondition::IF_NOT_EQUAL));
}

} // namespace codegen
