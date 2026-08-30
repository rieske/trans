#include "gtest/gtest.h"

#include "ast/AbstractSyntaxTreeBuilderContext.h"
#include "ast/ContextualSyntaxNodeBuilder.h"
#include "ast/CSNB_Internal.h"
#include "ast/TerminalSymbol.h"
#include "parser/BNFFileReader.h"
#include "scanner/LexicalSession.h"

#include "ResourceHelpers.h"

#include <stdexcept>
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

    const auto* intType = production(grammar, "<type_spec>", { "int" });
    const auto* unitPostfix = production(grammar, "<postfix_exp>", { "<primary_exp>" });
    ASSERT_NE(intType, nullptr);
    ASSERT_NE(unitPostfix, nullptr);

    EXPECT_NO_THROW(builder.updateContext(*unitPostfix, context));

    context.pushTerminal({ "int", "int", { "t.c", 1 } });
    builder.updateContext(*intType, context);
    EXPECT_TRUE(context.hasTypeSpecifier());

    EXPECT_THROW(builder.updateContext(grammar.getTopRule(), context), std::runtime_error);
}

// Remaining type stubs still throw; integer type-specs are implemented (Phase 1).
TEST(CSNBCreators, notImplementedYetFactoryThrows) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context{session};
    auto stub = ast::notImplementedYet("feature X");
    EXPECT_THROW(stub(context), std::runtime_error);
}

TEST(CSNBCreators, doNothingIsNoOp) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context{session};
    EXPECT_NO_THROW(ast::doNothing(context));
}

} // namespace
