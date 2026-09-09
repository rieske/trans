#include "CodeGeneratingVisitor.h"
#include "ast/AstNodes.h"

#include <stdexcept>

#include "codegen/IrBuilders.h"

namespace codegen {

void CodeGeneratingVisitor::visit(ast::JumpStatement& statement) {
    if (!statement.getJumpTo(store_)) {
        throw std::runtime_error { "JumpStatement has no target label" };
    }
    emit(ir::jump(id(*statement.getJumpTo(store_))));
}

void CodeGeneratingVisitor::visit(ast::SwitchStatement& statement) {
    statement.expression->accept(*this);

    const int switchResult = id(*statement.expression->getResultSymbol(store_));
    const int caseTemp = id(*statement.getCaseTemp(store_));

    for (auto* caseLabel : statement.getCases()) {
        emit(ir::assignConstant(
                id(std::to_string(caseLabel->getCaseValue())), caseTemp));
        emit(ir::valueCompare(switchResult, caseTemp));
        emit(ir::jump(id(*caseLabel->getLabel(store_)), JumpCondition::IF_EQUAL));
    }

    if (statement.getDefaultLabel()) {
        emit(ir::jump(id(*statement.getDefaultLabel()->getLabel(store_))));
    } else {
        emit(ir::jump(id(*statement.getExitLabel(store_))));
    }

    statement.body->accept(*this);
    emit(ir::label(id(*statement.getExitLabel(store_))));
}

void CodeGeneratingVisitor::visit(ast::CaseLabel& statement) {
    emit(ir::label(id(*statement.getLabel(store_))));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::DefaultLabel& statement) {
    emit(ir::label(id(*statement.getLabel(store_))));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::GotoStatement& statement) {
    if (!statement.getTarget(store_)) {
        throw std::runtime_error { "GotoStatement has no target label" };
    }
    emit(ir::jump(id(*statement.getTarget(store_))));
}

void CodeGeneratingVisitor::visit(ast::LabeledStatement& statement) {
    if (!statement.getLabel(store_)) {
        throw std::runtime_error { "LabeledStatement has no label" };
    }
    emit(ir::label(id(*statement.getLabel(store_))));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::ReturnStatement& statement) {
    if (!statement.returnExpression) {
        emit(ir::voidReturn());
        return;
    }
    statement.returnExpression->accept(*this);
    emit(ir::ret(convertedResult(*statement.returnExpression)));
}

void CodeGeneratingVisitor::visit(ast::ExpressionStatement& statement) {
    statement.expression->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::NullStatement& statement) {
    (void)statement;
}

void CodeGeneratingVisitor::visit(ast::IfStatement& statement) {
    statement.testExpression->accept(*this);

    emit(ir::zeroCompare(convertedResult(*statement.testExpression)));
    emit(ir::jump(id(*statement.getFalsyLabel(store_)), JumpCondition::IF_EQUAL));

    statement.body->accept(*this);
    if (statement.elseBody) {
        emit(ir::jump(id(*statement.getExitLabel(store_))));
        emit(ir::label(id(*statement.getFalsyLabel(store_))));
        statement.elseBody->accept(*this);
        emit(ir::label(id(*statement.getExitLabel(store_))));
        return;
    }
    emit(ir::label(id(*statement.getFalsyLabel(store_))));
}

void CodeGeneratingVisitor::visit(ast::LoopStatement& loop) {
    if (loop.header->bodyBeforeTest()) {
        // do { body } while (cond); - header visit emits the trailing test + branch.
        emit(ir::label(id(*loop.header->getLoopEntry(store_))));
        loop.body->accept(*this);
        emit(ir::label(id(*loop.header->getLoopContinue(store_))));
        loop.header->accept(*this);
        emit(ir::label(id(*loop.header->getLoopExit(store_))));
        return;
    }

    loop.header->accept(*this);
    loop.body->accept(*this);
    // continue target: for-loops place a label before the increment; while reuses entry.
    if (loop.header->getLoopContinue(store_)
            && loop.header->getLoopContinue(store_)->getName() != loop.header->getLoopEntry(store_)->getName()) {
        emit(ir::label(id(*loop.header->getLoopContinue(store_))));
    }
    if (loop.header->increment) {
        loop.header->increment->accept(*this);
    }

    emit(ir::jump(id(*loop.header->getLoopEntry(store_))));
    emit(ir::label(id(*loop.header->getLoopExit(store_))));
}

void CodeGeneratingVisitor::visit(ast::ForLoopHeader& loopHeader) {
    loopHeader.initialization.accept(*this);

    emit(ir::label(id(*loopHeader.getLoopEntry(store_))));
    if (loopHeader.clause) {
        loopHeader.clause->accept(*this);
        emit(ir::zeroCompare(convertedResult(*loopHeader.clause)));
        emit(ir::jump(id(*loopHeader.getLoopExit(store_)), JumpCondition::IF_EQUAL));
    }
}

void CodeGeneratingVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    emit(ir::label(id(*loopHeader.getLoopEntry(store_))));
    loopHeader.clause->accept(*this);
    emit(ir::zeroCompare(convertedResult(*loopHeader.clause)));
    emit(ir::jump(id(*loopHeader.getLoopExit(store_)), JumpCondition::IF_EQUAL));
}

void CodeGeneratingVisitor::visit(ast::DoWhileLoopHeader& loopHeader) {
    // Invoked after the body and continue label (see visit(LoopStatement)).
    loopHeader.clause->accept(*this);
    emit(ir::zeroCompare(convertedResult(*loopHeader.clause)));
    emit(ir::jump(id(*loopHeader.getLoopEntry(store_)), JumpCondition::IF_NOT_EQUAL));
}

} // namespace codegen
