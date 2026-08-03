#include "CodeGeneratingVisitor.h"
#include "CodeGeneratingVisitorInternal.h"
#include "Instruction.h"

#include <stdexcept>

#include "ast/Expression.h"

namespace codegen {




void CodeGeneratingVisitor::visit(ast::JumpStatement& statement) {
    emit(ir::jump(store_.label(&statement, symbols::LabelSlot::Target)->getName()));
}

void CodeGeneratingVisitor::visit(ast::ReturnStatement& statement) {
    std::string returnSymbol = generateExpression(*statement.returnExpression);
    emit(ir::ret(returnSymbol));
}

void CodeGeneratingVisitor::visit(ast::VoidReturnStatement &statement) { emit(ir::voidReturn()); }

void CodeGeneratingVisitor::visit(ast::IfStatement& statement) {
    generateExpression(*statement.testExpression);

    emit(ir::zeroCompare(statement.testExpression->getResultSymbol(store_)->getName()));
    emit(ir::jump(store_.label(&statement, symbols::LabelSlot::Falsy)->getName(), JumpCondition::IF_EQUAL));

    statement.body->accept(*this);

    emit(ir::label(store_.label(&statement, symbols::LabelSlot::Falsy)->getName()));
}

void CodeGeneratingVisitor::visit(ast::IfElseStatement& statement) {
    generateExpression(*statement.testExpression);

    emit(ir::zeroCompare(statement.testExpression->getResultSymbol(store_)->getName()));
    emit(ir::jump(store_.label(&statement, symbols::LabelSlot::Falsy)->getName(), JumpCondition::IF_EQUAL));

    statement.truthyBody->accept(*this);
    emit(ir::jump(store_.label(&statement, symbols::LabelSlot::Exit)->getName()));
    emit(ir::label(store_.label(&statement, symbols::LabelSlot::Falsy)->getName()));

    statement.falsyBody->accept(*this);
    emit(ir::label(store_.label(&statement, symbols::LabelSlot::Exit)->getName()));
}

void CodeGeneratingVisitor::visit(ast::LoopStatement& loop) {
    if (loop.header->bodyBeforeTest()) {
        // do { body } while (cond); — body first, then test at continue label.
        emit(ir::label(store_.label(loop.header.get(), symbols::LabelSlot::LoopEntry)->getName()));
        loop.body->accept(*this);
        emit(ir::label(store_.label(loop.header.get(), symbols::LabelSlot::LoopContinue)->getName()));
        if (auto* test = loop.header->testExpression()) {
            generateExpression(*test);
            emit(ir::zeroCompare(test->getResultSymbol(store_)->getName()));
            emit(ir::jump(
                    store_.label(loop.header.get(), symbols::LabelSlot::LoopEntry)->getName(), JumpCondition::IF_NOT_EQUAL));
        }
        emit(ir::label(store_.label(loop.header.get(), symbols::LabelSlot::LoopExit)->getName()));
        return;
    }

    loop.header->accept(*this);
    loop.body->accept(*this);

    emit(ir::label(store_.label(loop.header.get(), symbols::LabelSlot::LoopContinue)->getName()));
    if (loop.header->increment) {
        generateExpression(*loop.header->increment);
    }

    emit(ir::jump(store_.label(loop.header.get(), symbols::LabelSlot::LoopEntry)->getName()));
    emit(ir::label(store_.label(loop.header.get(), symbols::LabelSlot::LoopExit)->getName()));
}

void CodeGeneratingVisitor::visit(ast::SwitchStatement& statement) {
    generateExpression(*statement.expression);

    auto switchResult = statement.expression->getResultSymbol(store_)->getName();
    auto caseTemp = store_.value(&statement, symbols::ValueSlot::CaseTemp)->getName();

    for (auto* caseLabel : statement.getCases()) {
        emit(ir::assignConstant(
                std::to_string(caseLabel->getCaseValue()), caseTemp));
        emit(ir::valueCompare(switchResult, caseTemp));
        emit(ir::jump(store_.label(caseLabel, symbols::LabelSlot::Primary)->getName(), JumpCondition::IF_EQUAL));
    }

    if (statement.getDefaultLabel()) {
        emit(ir::jump(store_.label(statement.getDefaultLabel(), symbols::LabelSlot::Primary)->getName()));
    } else {
        emit(ir::jump(store_.label(&statement, symbols::LabelSlot::Exit)->getName()));
    }

    statement.body->accept(*this);
    emit(ir::label(store_.label(&statement, symbols::LabelSlot::Exit)->getName()));
}

void CodeGeneratingVisitor::visit(ast::CaseLabel& statement) {
    emit(ir::label(store_.label(&statement, symbols::LabelSlot::Primary)->getName()));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::DefaultLabel& statement) {
    emit(ir::label(store_.label(&statement, symbols::LabelSlot::Primary)->getName()));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::GotoStatement& statement) {
    if (!store_.label(&statement, symbols::LabelSlot::Target)) {
        throw std::runtime_error { "GotoStatement has no target label" };
    }
    emit(ir::jump(store_.label(&statement, symbols::LabelSlot::Target)->getName()));
}

void CodeGeneratingVisitor::visit(ast::LabeledStatement& statement) {
    if (!store_.label(&statement, symbols::LabelSlot::Primary)) {
        throw std::runtime_error { "LabeledStatement has no label" };
    }
    emit(ir::label(store_.label(&statement, symbols::LabelSlot::Primary)->getName()));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::ForLoopHeader& loopHeader) {
    if (loopHeader.initialization) {
        if (loopHeader.isDeclarationInit()) {
            loopHeader.initialization->accept(*this);
        } else if (auto* initExpr = dynamic_cast<ast::Expression*>(loopHeader.initialization.get())) {
            generateExpression(*initExpr);
        }
    }

    emit(ir::label(store_.label(&loopHeader, symbols::LabelSlot::LoopEntry)->getName()));
    if (loopHeader.clause) {
        generateExpression(*loopHeader.clause);
        emit(ir::zeroCompare(loopHeader.clause->getResultSymbol(store_)->getName()));
        emit(ir::jump(store_.label(&loopHeader, symbols::LabelSlot::LoopExit)->getName(), JumpCondition::IF_EQUAL));
    }
}

void CodeGeneratingVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    emit(ir::label(store_.label(&loopHeader, symbols::LabelSlot::LoopEntry)->getName()));
    generateExpression(*loopHeader.clause);
    emit(ir::zeroCompare(loopHeader.clause->getResultSymbol(store_)->getName()));
    emit(ir::jump(store_.label(&loopHeader, symbols::LabelSlot::LoopExit)->getName(), JumpCondition::IF_EQUAL));
}

void CodeGeneratingVisitor::visit(ast::DoWhileLoopHeader& loopHeader) {
    // Clause is generated in visit(LoopStatement) after the body.
    (void)loopHeader;
}

} // namespace codegen
