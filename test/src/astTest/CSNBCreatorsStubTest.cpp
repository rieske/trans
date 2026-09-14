#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ast/AbstractSyntaxTreeBuilderContext.h"
#include "ast/Block.h"
#include "ast/ContextualSyntaxNodeBuilder.h"
#include "ast/CSNB_Internal.h"
#include "ast/Declaration.h"
#include "ast/DeclarationSpecifiers.h"
#include "ast/IdentifierExpression.h"
#include "ast/ReturnStatement.h"
#include "ast/TerminalSymbol.h"
#include "ast/TypeSpecifier.h"
#include "parser/BNFFileReader.h"
#include "scanner/LexicalSession.h"
#include "types/Type.h"
#include "util/Diagnostic.h"

#include "ResourceHelpers.h"

#include <exception>
#include <initializer_list>
#include <sstream>
#include <vector>

namespace {

parser::Grammar productGrammar() {
    parser::BNFFileReader reader;
    return reader.readGrammar(getResourcePath("configuration/grammar.bnf"));
}

template<typename Symbols>
std::vector<int> symbolIds(const parser::Grammar& grammar, const Symbols& rhs) {
    std::vector<int> symbols;
    symbols.reserve(rhs.size());
    for (const char* symbol : rhs) {
        symbols.push_back(grammar.symbolId(symbol));
    }
    return symbols;
}

const parser::Production* production(const parser::Grammar& grammar, const char* lhs,
        const std::vector<int>& symbols) {
    for (const auto& rule : grammar.getProductionsOfSymbol(grammar.symbolId(lhs))) {
        if (rule.producedSequence() == symbols) {
            return &rule;
        }
    }
    return nullptr;
}

const parser::Production* production(const parser::Grammar& grammar, const char* lhs,
        std::initializer_list<const char*> rhs) {
    return production(grammar, lhs, symbolIds(grammar, rhs));
}

const parser::Production* production(const parser::Grammar& grammar, const char* lhs,
        const std::vector<const char*>& rhs) {
    return production(grammar, lhs, symbolIds(grammar, rhs));
}

TEST(CSNBCreators, productGrammarRegistersKnownProductions) {
    const parser::Grammar grammar = productGrammar();
    const ast::ContextualSyntaxNodeBuilder builder { grammar };
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context { session };
    std::ostringstream logged;
    diag::Sink sink { logged };
    context.setSink(&sink);

    const auto* intType = production(grammar, "<type_spec>", { "int" });
    const auto* unitPostfix = production(grammar, "<postfix_exp>", { "<primary_exp>" });
    const auto* unaryAddr = production(grammar, "<unary_exp>", { "&", "<cast_exp>" });
    const auto* assignEq = production(grammar, "<assignment_exp>",
            { "<unary_exp>", "=", "<assignment_exp>" });
    const auto* caseConst = production(grammar, "<labeled_stat_matched>",
            { "case", "<conditional_exp>", ":", "<matched>" });
    ASSERT_NE(intType, nullptr);
    ASSERT_NE(unitPostfix, nullptr);
    ASSERT_NE(unaryAddr, nullptr);
    ASSERT_NE(assignEq, nullptr);
    ASSERT_NE(caseConst, nullptr);
    EXPECT_FALSE(grammar.trySymbolId("<const_exp>").has_value());
    EXPECT_FALSE(grammar.trySymbolId("<unary_operator>").has_value());
    EXPECT_FALSE(grammar.trySymbolId("<assignment_operator>").has_value());
    EXPECT_FALSE(grammar.trySymbolId("<stat_list>").has_value());

    EXPECT_NO_THROW(builder.updateContext(*unitPostfix, context));

    context.pushTerminal({ "int", { "t.c", 1 } });
    builder.updateContext(*intType, context);
    EXPECT_TRUE(context.hasTypeSpecifier());

    EXPECT_NO_THROW(builder.updateContext(grammar.getTopRule(), context));
    EXPECT_TRUE(context.failed());
    EXPECT_TRUE(sink.hasErrors());
    EXPECT_THAT(logged.str(), testing::HasSubstr("error: language construct not implemented yet"));
}

TEST(CSNBCreators, knrStubsReportOnSink) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context{session};
    std::ostringstream logged;
    diag::Sink sink { logged };
    context.setSink(&sink);
    EXPECT_NO_THROW(ast::knrIdentifierParameterLists(context));
    EXPECT_THAT(logged.str(), testing::HasSubstr("K&R identifier parameter lists is not implemented yet"));
    EXPECT_NO_THROW(ast::knrStyleFunctionDefinitions(context));
    EXPECT_THAT(logged.str(), testing::HasSubstr("K&R style function definitions is not implemented yet"));
    EXPECT_TRUE(context.failed());
    EXPECT_TRUE(sink.hasErrors());
}

enum class ForInit { None, Expression, Declaration };

std::vector<const char*> forProductionRhs(ForInit init, bool clause, bool increment, const char* end) {
    std::vector<const char*> rhs { "for", "(" };
    if (init == ForInit::Expression) {
        rhs.push_back("<exp>");
        rhs.push_back(";");
    } else if (init == ForInit::None) {
        rhs.push_back(";");
    } else {
        rhs.push_back("<decl>");
    }
    if (clause) {
        rhs.push_back("<exp>");
    }
    rhs.push_back(";");
    if (increment) {
        rhs.push_back("<exp>");
    }
    rhs.push_back(")");
    rhs.push_back(end);
    return rhs;
}

void pushDummyForStack(ast::AbstractSyntaxTreeBuilderContext& context, ForInit init,
        bool clause, bool increment) {
    const translation_unit::Context where { "t.c", 1 };
    context.pushStatement(std::make_unique<ast::Block>());
    if (init == ForInit::Expression) {
        context.pushExpression(std::make_unique<ast::IdentifierExpression>("i", where));
    } else if (init == ForInit::Declaration) {
        context.pushDeclaration(std::make_unique<ast::Declaration>(
                ast::DeclarationSpecifiers { ast::TypeSpecifier { type::signedInteger(), "int" } }));
    }
    if (clause) {
        context.pushExpression(std::make_unique<ast::IdentifierExpression>("c", where));
    }
    if (increment) {
        context.pushExpression(std::make_unique<ast::IdentifierExpression>("n", where));
    }
    const int terminals = (init == ForInit::Declaration) ? 4 : 5;
    for (int i = 0; i < terminals; ++i) {
        context.pushTerminal({ ";", where });
    }
}

void expectForCreatorBound(const parser::Grammar& grammar,
        const ast::ContextualSyntaxNodeBuilder& builder, ForInit init, bool clause,
        bool increment, const char* lhs, const char* end) {
    const auto* prod = production(grammar, lhs, forProductionRhs(init, clause, increment, end));
    EXPECT_NE(prod, nullptr) << lhs << " ending in " << end;
    if (prod == nullptr) {
        return;
    }
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context { session };
    std::ostringstream logged;
    diag::Sink sink { logged };
    context.setSink(&sink);
    pushDummyForStack(context, init, clause, increment);
    try {
        builder.updateContext(*prod, context);
    } catch (const std::exception& ex) {
        ADD_FAILURE() << lhs << " ending in " << end << ": " << ex.what();
        return;
    }
    EXPECT_THAT(logged.str(),
            testing::Not(testing::HasSubstr("language construct not implemented yet")));
    auto statement = context.popAsStatement();
    ASSERT_NE(statement, nullptr);
    EXPECT_EQ(statement->asBlock(), nullptr);
}

TEST(CSNBCreators, forLoopProductionsAreRegistered) {
    const parser::Grammar grammar = productGrammar();
    const ast::ContextualSyntaxNodeBuilder builder { grammar };
    for (const auto init : { ForInit::Expression, ForInit::None, ForInit::Declaration }) {
        for (const bool clause : { true, false }) {
            for (const bool increment : { true, false }) {
                expectForCreatorBound(grammar, builder, init, clause, increment,
                        "<iteration_stat_matched>", "<matched>");
                expectForCreatorBound(grammar, builder, init, clause, increment,
                        "<iteration_stat_unmatched>", "<unmatched>");
            }
        }
    }
}

TEST(CSNBCreators, doNothingIsNoOp) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context{session};
    EXPECT_NO_THROW(ast::doNothing(context));
}

TEST(BuilderContext, popBlockReturnsBlock) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context { session };
    context.pushStatement(std::make_unique<ast::Block>());
    auto block = context.popBlock();
    ASSERT_NE(block, nullptr);
}

TEST(BuilderContext, popBlockRejectsNonBlockStatement) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context { session };
    context.pushStatement(std::make_unique<ast::ReturnStatement>());
    EXPECT_EQ(context.popBlock(), nullptr);
}

} // namespace
