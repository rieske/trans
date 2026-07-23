#include "CodeGeneratingVisitor.h"
#include "CodeGeneratingVisitorInternal.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "semantic_analyzer/ValueEntry.h"
#include "semantic_analyzer/LabelEntry.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "ObjectAbi.h"
#include "FrameLayout.h"

#include "quadruples/Assign.h"
#include "quadruples/Argument.h"
#include "quadruples/Call.h"
#include "quadruples/Retrieve.h"
#include "quadruples/AssignConstant.h"
#include "quadruples/Inc.h"
#include "quadruples/Dec.h"
#include "quadruples/AddressOf.h"
#include "quadruples/FunctionAddress.h"
#include "quadruples/Dereference.h"
#include "quadruples/UnaryMinus.h"
#include "quadruples/BitwiseNot.h"
#include "quadruples/ValueCompare.h"
#include "quadruples/ZeroCompare.h"
#include "quadruples/Jump.h"
#include "quadruples/Label.h"
#include "quadruples/Add.h"
#include "quadruples/Sub.h"
#include "quadruples/Mul.h"
#include "quadruples/Div.h"
#include "quadruples/Mod.h"
#include "quadruples/And.h"
#include "quadruples/Or.h"
#include "quadruples/Xor.h"
#include "quadruples/Return.h"
#include "quadruples/VoidReturn.h"
#include "quadruples/LvalueAssign.h"
#include "quadruples/StartProcedure.h"
#include "quadruples/EndProcedure.h"
#include "quadruples/Shl.h"
#include "quadruples/Shr.h"
#include "quadruples/FieldAccess.h"
#include "quadruples/IndexAddress.h"
#include "quadruples/Truncate.h"
#include "quadruples/BuiltinOp.h"
#include "quadruples/VaOp.h"
#include "ast/MemberAccess.h"
#include "ast/IdentifierExpression.h"
#include "ast/FunctionCall.h"
#include "ast/Expression.h"

namespace codegen {




void CodeGeneratingVisitor::visit(ast::JumpStatement& statement) {
    instructions.push_back(std::make_unique<Jump>(statement.getTarget()->getName()));
}

void CodeGeneratingVisitor::visit(ast::ReturnStatement& statement) {
    std::string returnSymbol = generateExpression(*statement.returnExpression);
    instructions.push_back(std::make_unique<Return>(returnSymbol));
}

void CodeGeneratingVisitor::visit(ast::VoidReturnStatement &statement) { instructions.push_back(std::make_unique<VoidReturn>()); }

void CodeGeneratingVisitor::visit(ast::IfStatement& statement) {
    generateExpression(*statement.testExpression);

    instructions.push_back(std::make_unique<ZeroCompare>(statement.testExpression->getResultSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(statement.getFalsyLabel()->getName(), JumpCondition::IF_EQUAL));

    statement.body->accept(*this);

    instructions.push_back(std::make_unique<Label>(statement.getFalsyLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::IfElseStatement& statement) {
    generateExpression(*statement.testExpression);

    instructions.push_back(std::make_unique<ZeroCompare>(statement.testExpression->getResultSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(statement.getFalsyLabel()->getName(), JumpCondition::IF_EQUAL));

    statement.truthyBody->accept(*this);
    instructions.push_back(std::make_unique<Jump>(statement.getExitLabel()->getName()));
    instructions.push_back(std::make_unique<Label>(statement.getFalsyLabel()->getName()));

    statement.falsyBody->accept(*this);
    instructions.push_back(std::make_unique<Label>(statement.getExitLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::LoopStatement& loop) {
    auto* doWhileHeader = dynamic_cast<ast::DoWhileLoopHeader*>(loop.header.get());
    if (doWhileHeader) {
        // do { body } while (cond); - body first, then test
        instructions.push_back(std::make_unique<Label>(loop.header->getLoopEntry()->getName()));
        loop.body->accept(*this);
        instructions.push_back(std::make_unique<Label>(loop.header->getLoopContinue()->getName()));
        generateExpression(*doWhileHeader->clause);
        instructions.push_back(std::make_unique<ZeroCompare>(doWhileHeader->clause->getResultSymbol()->getName()));
        instructions.push_back(std::make_unique<Jump>(loop.header->getLoopEntry()->getName(), JumpCondition::IF_NOT_EQUAL));
        instructions.push_back(std::make_unique<Label>(loop.header->getLoopExit()->getName()));
        return;
    }

    loop.header->accept(*this);
    loop.body->accept(*this);

    instructions.push_back(std::make_unique<Label>(loop.header->getLoopContinue()->getName()));
    if (loop.header->increment) {
        generateExpression(*loop.header->increment);
    }

    instructions.push_back(std::make_unique<Jump>(loop.header->getLoopEntry()->getName()));
    instructions.push_back(std::make_unique<Label>(loop.header->getLoopExit()->getName()));
}

void CodeGeneratingVisitor::visit(ast::SwitchStatement& statement) {
    generateExpression(*statement.expression);

    auto switchResult = statement.expression->getResultSymbol()->getName();
    auto caseTemp = statement.getCaseTemp()->getName();

    for (auto* caseLabel : statement.getCases()) {
        instructions.push_back(std::make_unique<AssignConstant>(
                std::to_string(caseLabel->getCaseValue()), caseTemp));
        instructions.push_back(std::make_unique<ValueCompare>(switchResult, caseTemp));
        instructions.push_back(std::make_unique<Jump>(caseLabel->getLabel()->getName(), JumpCondition::IF_EQUAL));
    }

    if (statement.getDefaultLabel()) {
        instructions.push_back(std::make_unique<Jump>(statement.getDefaultLabel()->getLabel()->getName()));
    } else {
        instructions.push_back(std::make_unique<Jump>(statement.getExitLabel()->getName()));
    }

    statement.body->accept(*this);
    instructions.push_back(std::make_unique<Label>(statement.getExitLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::CaseLabel& statement) {
    instructions.push_back(std::make_unique<Label>(statement.getLabel()->getName()));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::DefaultLabel& statement) {
    instructions.push_back(std::make_unique<Label>(statement.getLabel()->getName()));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::GotoStatement& statement) {
    instructions.push_back(std::make_unique<Jump>(statement.getTarget()->getName()));
}

void CodeGeneratingVisitor::visit(ast::LabeledStatement& statement) {
    instructions.push_back(std::make_unique<Label>(statement.getLabel()->getName()));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::ForLoopHeader& loopHeader) {
    if (loopHeader.declarationInit) {
        loopHeader.declarationInit->accept(*this);
    }
    if (loopHeader.initialization) {
        generateExpression(*loopHeader.initialization);
    }

    instructions.push_back(std::make_unique<Label>(loopHeader.getLoopEntry()->getName()));
    if (loopHeader.clause) {
        generateExpression(*loopHeader.clause);
        instructions.push_back(std::make_unique<ZeroCompare>(loopHeader.clause->getResultSymbol()->getName()));
        instructions.push_back(std::make_unique<Jump>(loopHeader.getLoopExit()->getName(), JumpCondition::IF_EQUAL));
    }
}

void CodeGeneratingVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    instructions.push_back(std::make_unique<Label>(loopHeader.getLoopEntry()->getName()));
    generateExpression(*loopHeader.clause);
    instructions.push_back(std::make_unique<ZeroCompare>(loopHeader.clause->getResultSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(loopHeader.getLoopExit()->getName(), JumpCondition::IF_EQUAL));
}

void CodeGeneratingVisitor::visit(ast::DoWhileLoopHeader& loopHeader) {
    // Clause is generated in visit(LoopStatement) after the body.
    (void)loopHeader;
}

} // namespace codegen
