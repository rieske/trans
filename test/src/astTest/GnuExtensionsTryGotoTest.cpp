#include "gtest/gtest.h"

#include "ast/GnuExtensions.h"
#include "parser/BNFFileReader.h"
#include "parser/ParsingTable.h"
#include "parser/TokenStream.h"
#include "scanner/LexicalSession.h"
#include "scanner/Token.h"

#include "ResourceHelpers.h"

#include <vector>

namespace {

parser::Grammar productGrammar() {
    parser::BNFFileReader reader;
    return reader.readGrammar(getResourcePath("configuration/grammar.bnf"));
}

parser::TokenStream streamFor(std::vector<scanner::Token>& tokens, std::size_t& index,
        scanner::LexicalSession& session, const parser::Grammar& grammar) {
    return parser::TokenStream {
            [&]() { return tokens[index++]; },
            session,
            grammar };
}

} // namespace

TEST(GnuExtensionsTryGoto, ordinaryTokensDoNotTakeTheExtensionEdge) {
    const parser::Grammar grammar = productGrammar();
    const parser::ParsingTable table { &grammar };
    ast::GnuExtensions extensions;
    scanner::LexicalSession session;
    std::vector<scanner::Token> tokens {
            { "int", "int", { "t.c", 1 } },
            { scanner::Token::END, scanner::Token::END, { "t.c", 1 } },
    };
    std::size_t index = 0;
    parser::TokenStream stream = streamFor(tokens, index, session, grammar);
    EXPECT_FALSE(extensions.tryGoto(0, stream, table).has_value());
}

TEST(GnuExtensionsTryGoto, statementExpressionProbesPrimaryExpGoto) {
    const parser::Grammar grammar = productGrammar();
    const parser::ParsingTable table { &grammar };
    ast::GnuExtensions extensions;
    scanner::LexicalSession session;
    std::vector<scanner::Token> tokens {
            { "(", "(", { "t.c", 1 } },
            { "{", "{", { "t.c", 1 } },
            { scanner::Token::END, scanner::Token::END, { "t.c", 1 } },
    };
    std::size_t index = 0;
    parser::TokenStream stream = streamFor(tokens, index, session, grammar);
    EXPECT_EQ(extensions.tryGoto(0, stream, table),
            table.tryGoTo(0, *grammar.trySymbolId("<primary_exp>")));
    EXPECT_EQ(stream.getCurrentToken().id, "(");
}

TEST(GnuExtensionsTryGoto, int128ProbesTypeSpecGoto) {
    const parser::Grammar grammar = productGrammar();
    const parser::ParsingTable table { &grammar };
    ast::GnuExtensions extensions;
    scanner::LexicalSession session;
    std::vector<scanner::Token> tokens {
            { "id", "__int128", { "t.c", 1 } },
            { scanner::Token::END, scanner::Token::END, { "t.c", 1 } },
    };
    std::size_t index = 0;
    parser::TokenStream stream = streamFor(tokens, index, session, grammar);
    EXPECT_EQ(extensions.tryGoto(0, stream, table),
            table.tryGoTo(0, *grammar.trySymbolId("<type_spec>")));
}

TEST(GnuExtensionsTryGoto, builtinVaArgProbesUnaryExpGoto) {
    const parser::Grammar grammar = productGrammar();
    const parser::ParsingTable table { &grammar };
    ast::GnuExtensions extensions;
    scanner::LexicalSession session;
    std::vector<scanner::Token> tokens {
            { "id", "__builtin_va_arg", { "t.c", 1 } },
            { scanner::Token::END, scanner::Token::END, { "t.c", 1 } },
    };
    std::size_t index = 0;
    parser::TokenStream stream = streamFor(tokens, index, session, grammar);
    EXPECT_EQ(extensions.tryGoto(0, stream, table),
            table.tryGoTo(0, *grammar.trySymbolId("<unary_exp>")));
}

TEST(GnuExtensionsTryGoto, isTypeExtensionTokenOnlyForInt128Spellings) {
    ast::GnuExtensions extensions;
    EXPECT_TRUE(extensions.isTypeExtensionToken({ "id", "__int128", { "t.c", 1 } }));
    EXPECT_TRUE(extensions.isTypeExtensionToken({ "id", "__int128_t", { "t.c", 1 } }));
    EXPECT_TRUE(extensions.isTypeExtensionToken({ "id", "__uint128_t", { "t.c", 1 } }));
    EXPECT_FALSE(extensions.isTypeExtensionToken({ "id", "x", { "t.c", 1 } }));
    EXPECT_FALSE(extensions.isTypeExtensionToken({ "int", "int", { "t.c", 1 } }));
}
