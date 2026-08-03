#include "CSNB_Internal.h"

#include "Block.h"
#include "CaseLabel.h"
#include "DefaultLabel.h"
#include "DoWhileLoopHeader.h"
#include "ForLoopHeader.h"
#include "FunctionDefinition.h"
#include "GotoStatement.h"
#include "IfElseStatement.h"
#include "IfStatement.h"
#include "JumpStatement.h"
#include "LabeledStatement.h"
#include "LoopStatement.h"
#include "ReturnStatement.h"
#include "SwitchStatement.h"
#include "VoidReturnStatement.h"
#include "WhileLoopHeader.h"
#include "types/Type.h"

#include <functional>

namespace ast {
namespace csnb {

namespace {

void ifStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<IfStatement>(context.popExpression(), context.popStatement()));
}

void ifElseStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    auto falsyStatement = context.popStatement();
    auto truthyStatement = context.popStatement();
    context.pushStatement(std::make_unique<IfElseStatement>(context.popExpression(), std::move(truthyStatement), std::move(falsyStatement)));
}

void switchStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.popTerminal(); // switch
    auto body = context.popStatement();
    context.pushStatement(std::make_unique<SwitchStatement>(context.popExpression(), std::move(body)));
}

void caseLabel(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // :
    context.popTerminal(); // case
    auto statement = context.popStatement();
    context.pushStatement(std::make_unique<CaseLabel>(context.popExpression(), std::move(statement)));
}

void defaultLabel(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // :
    auto defaultKeyword = context.popTerminal(); // default
    auto statement = context.popStatement();
    context.pushStatement(std::make_unique<DefaultLabel>(defaultKeyword, std::move(statement)));
}

void namedLabel(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // :
    auto labelName = context.popTerminal(); // id
    auto statement = context.popStatement();
    context.pushStatement(std::make_unique<LabeledStatement>(labelName, std::move(statement)));
}

void gotoStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ;
    auto labelName = context.popTerminal(); // id
    auto gotoKeyword = context.popTerminal(); // goto
    context.pushStatement(std::make_unique<GotoStatement>(gotoKeyword, labelName));
}

void whileLoopStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    auto loopHeader = std::make_unique<WhileLoopHeader>(context.popExpression());
    auto body = context.popStatement();
    context.pushStatement(std::make_unique<LoopStatement>(std::move(loopHeader), std::move(body)));
}

void doWhileLoopStatement(AbstractSyntaxTreeBuilderContext& context) {
    // Production: 'do' <stat> 'while' '(' <exp> ')' ';'
    context.popTerminal(); // ;
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.popTerminal(); // while
    context.popTerminal(); // do
    auto clause = context.popExpression();
    auto body = context.popStatement();
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
    context.pushStatement(std::make_unique<VoidReturnStatement>());
}
void emptyCompound(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<Block>(std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> {}));
}

void blockItemListCompound(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // }
    context.popTerminal(); // {
    context.pushStatement(std::make_unique<Block>(context.popStatementList()));
}

void blockItemDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    // Treat a declaration as a block item on the statement/item list.
    context.pushStatement(context.popDeclaration());
}

void blockItemStatement(AbstractSyntaxTreeBuilderContext& context) {
    // Statement already on the statement stack from <stat>.
    (void)context;
}

// Reuse the statement-list stack as an ordered block-item list.
void blockItemListFromItem(AbstractSyntaxTreeBuilderContext& context) {
    context.newStatementList(context.popStatement());
}

void blockItemListAppend(AbstractSyntaxTreeBuilderContext& context) {
    context.addToStatementList(context.popStatement());
}

void expressionStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushStatement(context.popExpression());
}

void emptyStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    // Null statement `;` must still occupy a block-item slot.
    context.pushStatement(std::make_unique<Block>(std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> {}));
}

void functionDefinition(AbstractSyntaxTreeBuilderContext& context) {
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    auto declarator = context.popDeclarator();
    auto statement = context.popStatement();
    context.pushStatement(std::make_unique<FunctionDefinition>(std::move(declarationSpecifiers), std::move(declarator), std::move(statement)));
}

void defaultReturnTypeFunctionDefinition(AbstractSyntaxTreeBuilderContext& context) {
    DeclarationSpecifiers defaultReturnTypeSpecifiers { TypeSpecifier { type::signedInteger(), "int" } };
    context.pushStatement(std::make_unique<FunctionDefinition>(defaultReturnTypeSpecifiers, context.popDeclarator(), context.popStatement()));
}

void externalFunctionDefinition(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExternalDeclaration(context.popStatement());
}

void externalDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExternalDeclaration(context.popDeclaration());
}

void translationUnit(AbstractSyntaxTreeBuilderContext& context) {
    context.addToTranslationUnit(context.popExternalDeclaration());
}

void addToTranslationUnit(AbstractSyntaxTreeBuilderContext& context) {
    context.addToTranslationUnit(context.popExternalDeclaration());
}

} // namespace


void loopJumpStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // semicolon
    context.pushStatement(std::make_unique<JumpStatement>(context.popTerminal()));
}

void registerStatementProductions(const parser::Grammar& grammar, NodeCreatorRegistry& nodeCreatorRegistry) {
    int s_open_paren = grammar.symbolId("(");
    int s_close_paren = grammar.symbolId(")");
    int s_exp = grammar.symbolId("<exp>");
    int s_semicolon = grammar.symbolId(";");
    int s_identifier = grammar.symbolId("id");
    int s_const_exp = grammar.symbolId("<const_exp>");
    int s_decl_specs = grammar.symbolId("<decl_specs>");
    int s_declarator = grammar.symbolId("<declarator>");
    int s_decl = grammar.symbolId("<decl>");
    int s_open_brace = grammar.symbolId("{");
    int s_close_brace = grammar.symbolId("}");

    int s_exp_stat = grammar.symbolId("<exp_stat>");
    nodeCreatorRegistry[s_exp_stat][{ s_exp, s_semicolon }] = expressionStatement;
    nodeCreatorRegistry[s_exp_stat][{ s_semicolon }] = emptyStatement;

    int s_matched = grammar.symbolId("<matched>");
    int s_unmatched = grammar.symbolId("<unmatched>");
    int s_stat = grammar.symbolId("<stat>");
    int s_compound_stat = grammar.symbolId("<compound_stat>");
    int s_jump_stat = grammar.symbolId("<jump_stat>");
    int s_if = grammar.symbolId("if");
    nodeCreatorRegistry[s_matched][{s_if, s_open_paren, s_exp, s_close_paren, s_matched, grammar.symbolId("else"), s_matched }] = ifElseStatement;
    nodeCreatorRegistry[s_unmatched][{s_if, s_open_paren, s_exp, s_close_paren, s_stat }] = ifStatement;
    nodeCreatorRegistry[s_unmatched][{s_if, s_open_paren, s_exp, s_close_paren, s_matched, grammar.symbolId("else"), s_unmatched }] = ifElseStatement;
    int s_switch = grammar.symbolId("switch");
    nodeCreatorRegistry[s_matched][{ s_switch, s_open_paren, s_exp, s_close_paren, s_matched }] = switchStatement;
    nodeCreatorRegistry[s_unmatched][{ s_switch, s_open_paren, s_exp, s_close_paren, s_unmatched }] = switchStatement;

    int s_labeled_stat_matched = grammar.symbolId("<labeled_stat_matched>");
    int s_labeled_stat_unmatched = grammar.symbolId("<labeled_stat_unmatched>");
    int s_case = grammar.symbolId("case");
    int s_default = grammar.symbolId("default");
    int s_colon = grammar.symbolId(":");
    nodeCreatorRegistry[s_matched][{ s_labeled_stat_matched }] = doNothing;
    nodeCreatorRegistry[s_unmatched][{ s_labeled_stat_unmatched }] = doNothing;
    nodeCreatorRegistry[s_labeled_stat_matched][{ s_case, s_const_exp, s_colon, s_matched }] = caseLabel;
    nodeCreatorRegistry[s_labeled_stat_unmatched][{ s_case, s_const_exp, s_colon, s_unmatched }] = caseLabel;
    nodeCreatorRegistry[s_labeled_stat_matched][{ s_default, s_colon, s_matched }] = defaultLabel;
    nodeCreatorRegistry[s_labeled_stat_unmatched][{ s_default, s_colon, s_unmatched }] = defaultLabel;
    nodeCreatorRegistry[s_labeled_stat_matched][{ s_identifier, s_colon, s_matched }] = namedLabel;
    nodeCreatorRegistry[s_labeled_stat_unmatched][{ s_identifier, s_colon, s_unmatched }] = namedLabel;

    nodeCreatorRegistry[s_matched][{ s_exp_stat }] = doNothing;
    nodeCreatorRegistry[s_matched][{ s_compound_stat }] = doNothing;
    nodeCreatorRegistry[s_matched][{ s_jump_stat }] = doNothing;

    nodeCreatorRegistry[s_stat][{ s_matched }] = doNothing;
    nodeCreatorRegistry[s_stat][{ s_unmatched }] = doNothing;

    int s_stat_list = grammar.symbolId("<stat_list>");
    nodeCreatorRegistry[s_stat_list][{ s_stat }] = statementList;
    nodeCreatorRegistry[s_stat_list][{ s_stat_list, s_stat }] = addToStatementList;

    int s_return = grammar.symbolId("return");
    nodeCreatorRegistry[s_jump_stat][{ grammar.symbolId("goto"), s_identifier, s_semicolon }] = gotoStatement;
    nodeCreatorRegistry[s_jump_stat][{ grammar.symbolId("continue"), s_semicolon }] = loopJumpStatement;
    nodeCreatorRegistry[s_jump_stat][{ grammar.symbolId("break"), s_semicolon }] = loopJumpStatement;
    nodeCreatorRegistry[s_jump_stat][{ s_return, s_exp, s_semicolon }] = returnExpressionStatement;
    nodeCreatorRegistry[s_jump_stat][{ s_return, s_semicolon }] = returnVoidStatement;

    int s_block_item = grammar.symbolId("<block_item>");
    int s_block_item_list = grammar.symbolId("<block_item_list>");
    int s_stat_for_block = grammar.symbolId("<stat>");
    nodeCreatorRegistry[s_block_item][{ s_decl }] = blockItemDeclaration;
    nodeCreatorRegistry[s_block_item][{ s_stat_for_block }] = blockItemStatement;
    nodeCreatorRegistry[s_block_item_list][{ s_block_item }] = blockItemListFromItem;
    nodeCreatorRegistry[s_block_item_list][{ s_block_item_list, s_block_item }] = blockItemListAppend;
    nodeCreatorRegistry[s_compound_stat][{ s_open_brace, s_block_item_list, s_close_brace }] = blockItemListCompound;
    nodeCreatorRegistry[s_compound_stat][{ s_open_brace, s_close_brace }] = emptyCompound;

    int s_function_definition = grammar.symbolId("<function_definition>");
    int s_decl_list = grammar.symbolId("<decl_list>");
    nodeCreatorRegistry[s_function_definition][{ s_decl_specs, s_declarator, s_compound_stat }] = functionDefinition;
    nodeCreatorRegistry[s_function_definition][{ s_declarator, s_compound_stat }] = defaultReturnTypeFunctionDefinition;
    // K&R definitions: `int f(a) int a; { ... }` (parameter decls between declarator and body).
    nodeCreatorRegistry[s_function_definition][{ s_decl_specs, s_declarator, s_decl_list, s_compound_stat }] =
            notImplementedYet("K&R style function definitions");
    nodeCreatorRegistry[s_function_definition][{ s_declarator, s_decl_list, s_compound_stat }] =
            notImplementedYet("K&R style function definitions");

    int s_external_decl = grammar.symbolId("<external_decl>");
    nodeCreatorRegistry[s_external_decl][{ s_function_definition }] = externalFunctionDefinition;
    nodeCreatorRegistry[s_external_decl][{ s_decl }] = externalDeclaration;

    int s_translation_unit = grammar.symbolId("<translation_unit>");
    nodeCreatorRegistry[s_translation_unit][{ s_external_decl }] = translationUnit;
    nodeCreatorRegistry[s_translation_unit][{ s_translation_unit, s_external_decl }] = addToTranslationUnit;

    int s_iteration_stat_matched = grammar.symbolId("<iteration_stat_matched>");
    int s_iteration_stat_unmatched = grammar.symbolId("<iteration_stat_unmatched>");
    nodeCreatorRegistry[s_matched][{ s_iteration_stat_matched }] = doNothing;
    nodeCreatorRegistry[s_unmatched][{ s_iteration_stat_unmatched }] = doNothing;

    int s_while = grammar.symbolId("while");
    int s_for = grammar.symbolId("for");
    nodeCreatorRegistry[s_iteration_stat_matched][{ s_while, s_open_paren, s_exp, s_close_paren, s_matched }] = whileLoopStatement;
    nodeCreatorRegistry[s_iteration_stat_unmatched][{ s_while, s_open_paren, s_exp, s_close_paren, s_unmatched }] = whileLoopStatement;

    int s_do = grammar.symbolId("do");
    nodeCreatorRegistry[s_iteration_stat_matched][{
            s_do, s_matched, s_while, s_open_paren, s_exp, s_close_paren, s_semicolon
    }] = doWhileLoopStatement;
    nodeCreatorRegistry[s_iteration_stat_unmatched][{
            s_do, s_unmatched, s_while, s_open_paren, s_exp, s_close_paren, s_semicolon
    }] = doWhileLoopStatement;

    auto forLoop = [](bool hasInit, bool hasClause, bool hasIncrement) {
        return [=](AbstractSyntaxTreeBuilderContext& context) {
            for (int i = 0; i < 5; ++i) {
                context.popTerminal();  // for ( ; ; ) punctuation
            }
            auto increment = hasIncrement ? context.popExpression() : nullptr;
            auto clause = hasClause ? context.popExpression() : nullptr;
            std::unique_ptr<AbstractSyntaxTreeNode> initialization =
                    hasInit ? std::unique_ptr<AbstractSyntaxTreeNode>(context.popExpression()) : nullptr;
            auto loopHeader = std::make_unique<ForLoopHeader>(
                    std::move(initialization), std::move(clause), std::move(increment));
            auto body = context.popStatement();
            context.pushStatement(std::make_unique<LoopStatement>(std::move(loopHeader), std::move(body)));
        };
    };
    // C99: for ( declaration ... ) - <decl> already includes its terminating ';'.
    // Production terminals are always: for ( ; )  (the decl's own ';' is consumed by <decl>).
    auto forLoopWithDecl = [](bool hasClause, bool hasIncrement) {
        return [=](AbstractSyntaxTreeBuilderContext& context) {
            for (int i = 0; i < 4; ++i) {
                context.popTerminal(); // for ( ; )
            }
            auto increment = hasIncrement ? context.popExpression() : nullptr;
            auto clause = hasClause ? context.popExpression() : nullptr;
            std::unique_ptr<AbstractSyntaxTreeNode> initialization = context.popDeclaration();
            auto loopHeader = std::make_unique<ForLoopHeader>(
                    std::move(initialization), std::move(clause), std::move(increment));
            auto body = context.popStatement();
            context.pushStatement(std::make_unique<LoopStatement>(std::move(loopHeader), std::move(body)));
        };
    };
    auto registerFor = [&](const std::vector<int>& prod, std::function<void(AbstractSyntaxTreeBuilderContext&)> creator) {
        nodeCreatorRegistry[s_iteration_stat_matched][prod] = creator;
        auto unmatchedProd = prod;
        unmatchedProd.back() = s_unmatched;
        nodeCreatorRegistry[s_iteration_stat_unmatched][unmatchedProd] = creator;
    };
    registerFor({ s_for, s_open_paren, s_exp, s_semicolon, s_exp, s_semicolon, s_exp, s_close_paren, s_matched }, forLoop(true,  true,  true));
    registerFor({ s_for, s_open_paren, s_exp, s_semicolon, s_exp, s_semicolon, s_close_paren, s_matched }, forLoop(true,  true,  false));
    registerFor({ s_for, s_open_paren, s_exp, s_semicolon, s_semicolon, s_exp, s_close_paren, s_matched }, forLoop(true,  false, true));
    registerFor({ s_for, s_open_paren, s_exp, s_semicolon, s_semicolon, s_close_paren, s_matched }, forLoop(true,  false, false));
    registerFor({ s_for, s_open_paren, s_semicolon, s_exp, s_semicolon, s_exp, s_close_paren, s_matched }, forLoop(false, true,  true));
    registerFor({ s_for, s_open_paren, s_semicolon, s_exp, s_semicolon, s_close_paren, s_matched }, forLoop(false, true,  false));
    registerFor({ s_for, s_open_paren, s_semicolon, s_semicolon, s_exp, s_close_paren, s_matched }, forLoop(false, false, true));
    registerFor({ s_for, s_open_paren, s_semicolon, s_semicolon, s_close_paren, s_matched }, forLoop(false, false, false));

    int s_decl_for = grammar.symbolId("<decl>");
    registerFor({ s_for, s_open_paren, s_decl_for, s_exp, s_semicolon, s_exp, s_close_paren, s_matched }, forLoopWithDecl(true, true));
    registerFor({ s_for, s_open_paren, s_decl_for, s_exp, s_semicolon, s_close_paren, s_matched }, forLoopWithDecl(true, false));
    registerFor({ s_for, s_open_paren, s_decl_for, s_semicolon, s_exp, s_close_paren, s_matched }, forLoopWithDecl(false, true));
    registerFor({ s_for, s_open_paren, s_decl_for, s_semicolon, s_close_paren, s_matched }, forLoopWithDecl(false, false));
}

} // namespace csnb
} // namespace ast
