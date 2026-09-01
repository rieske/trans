#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ast/AbstractSyntaxTreeBuilderContext.h"
#include "ast/ContextualSyntaxNodeBuilder.h"
#include "ast/CSNB_Internal.h"
#include "ast/TerminalSymbol.h"
#include "parser/BNFFileReader.h"
#include "scanner/LexicalSession.h"
#include "util/Diagnostic.h"

#include "ResourceHelpers.h"

#include <sstream>
#include <vector>

namespace {

parser::Grammar productGrammar() {
    parser::BNFFileReader reader;
    return reader.readGrammar(getResourcePath("configuration/grammar.bnf"));
}

const parser::Production* production(const parser::Grammar& grammar, const char* lhs,
        std::initializer_list<const char*> rhs) {
    std::vector<int> symbols;
    symbols.reserve(rhs.size());
    for (const char* symbol : rhs) {
        symbols.push_back(grammar.symbolId(symbol));
    }
    for (const auto& rule : grammar.getProductionsOfSymbol(grammar.symbolId(lhs))) {
        if (rule.producedSequence() == symbols) {
            return &rule;
        }
    }
    return nullptr;
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
    ASSERT_NE(intType, nullptr);
    ASSERT_NE(unitPostfix, nullptr);

    EXPECT_NO_THROW(builder.updateContext(*unitPostfix, context));

    context.pushTerminal({ "int", "int", { "t.c", 1 } });
    builder.updateContext(*intType, context);
    EXPECT_TRUE(context.hasTypeSpecifier());

    EXPECT_NO_THROW(builder.updateContext(grammar.getTopRule(), context));
    EXPECT_TRUE(context.failed());
    EXPECT_TRUE(sink.hasErrors());
    EXPECT_THAT(logged.str(), testing::HasSubstr("error: language construct not implemented yet"));
}

TEST(CSNBCreators, notImplementedYetReportsOnSink) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context{session};
    std::ostringstream logged;
    diag::Sink sink { logged };
    context.setSink(&sink);
    auto stub = ast::notImplementedYet("feature X");
    EXPECT_NO_THROW(stub(context));
    EXPECT_TRUE(context.failed());
    EXPECT_TRUE(sink.hasErrors());
    EXPECT_THAT(logged.str(), testing::HasSubstr("error: feature X is not implemented yet"));
}

TEST(CSNBCreators, doNothingIsNoOp) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context{session};
    EXPECT_NO_THROW(ast::doNothing(context));
}

} // namespace
