#include "CSNB_Internal.h"

#include "AbstractSyntaxTreeBuilderContext.h"
#include "Block.h"
#include "CaseLabel.h"
#include "DefaultLabel.h"
#include "DoWhileLoopHeader.h"
#include "GotoStatement.h"
#include "IfStatement.h"
#include "LabeledStatement.h"
#include "LoopStatement.h"
#include "NullStatement.h"
#include "ReturnStatement.h"
#include "SwitchStatement.h"
#include "WhileLoopHeader.h"

namespace ast {

void ifStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<IfStatement>(context.popExpression(), context.popAsStatement()));
}

void ifElseStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    auto falsyStatement = context.popAsStatement();
    auto truthyStatement = context.popAsStatement();
    context.pushStatement(std::make_unique<IfStatement>(
            context.popExpression(), std::move(truthyStatement), std::move(falsyStatement)));
}

void whileLoopStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    auto loopHeader = std::make_unique<WhileLoopHeader>(context.popExpression());
    auto body = context.popAsStatement();
    context.pushStatement(std::make_unique<LoopStatement>(std::move(loopHeader), std::move(body)));
}

void namedLabel(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // :
    auto labelName = context.popTerminal(); // id
    auto statement = context.popAsStatement();
    context.pushStatement(std::make_unique<LabeledStatement>(labelName, std::move(statement)));
}

void switchStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.popTerminal(); // switch
    auto body = context.popAsStatement();
    context.pushStatement(std::make_unique<SwitchStatement>(context.popExpression(), std::move(body)));
}

void caseLabel(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // :
    context.popTerminal(); // case
    auto statement = context.popAsStatement();
    context.pushStatement(std::make_unique<CaseLabel>(context.popExpression(), std::move(statement)));
}

void defaultLabel(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // :
    auto defaultKeyword = context.popTerminal(); // default
    auto statement = context.popAsStatement();
    context.pushStatement(std::make_unique<DefaultLabel>(defaultKeyword, std::move(statement)));
}

void gotoStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ;
    auto labelName = context.popTerminal(); // id
    auto gotoKeyword = context.popTerminal(); // goto
    context.pushStatement(std::make_unique<GotoStatement>(gotoKeyword, labelName));
}

void doWhileLoopStatement(AbstractSyntaxTreeBuilderContext& context) {
    // Production: 'do' <stat> 'while' '(' <exp> ')' ';'
    context.popTerminal(); // ;
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.popTerminal(); // while
    context.popTerminal(); // do
    auto clause = context.popExpression();
    auto body = context.popAsStatement();
    auto loopHeader = std::make_unique<DoWhileLoopHeader>(std::move(clause));
    context.pushStatement(std::make_unique<LoopStatement>(std::move(loopHeader), std::move(body)));
}

void statementList(AbstractSyntaxTreeBuilderContext& context) {
    context.newStatementList(context.popStatement());
}

void addToStatementList(AbstractSyntaxTreeBuilderContext& context) {
    context.addToStatementList(context.popStatement());
}

void returnExpressionStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<ReturnStatement>(context.popExpression()));
}

void returnVoidStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<ReturnStatement>());
}

void emptyCompound(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<Block>());
}

void blockItemListCompound(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // }
    context.popTerminal(); // {
    context.pushStatement(std::make_unique<Block>(context.popStatementList()));
}

void blockItemDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    // Declaration becomes a block item on the shared statement/item stack.
    context.pushStatement(context.popDeclaration());
}

void expressionStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushStatement(context.popExpression());
}

void emptyStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushStatement(std::make_unique<NullStatement>());
}


} // namespace ast
