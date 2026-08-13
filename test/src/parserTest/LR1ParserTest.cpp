#include "gtest/gtest.h"

#include "scanner/LexicalSession.h"
#include "gmock/gmock.h"

#include "ast/AbstractSyntaxTreeBuilder.h"
#include "ast/IdentifierExpression.h"
#include "ast/TypeCast.h"
#include "parser/LR1Parser.h"
#include "parser/FilePersistedParsingTable.h"
#include "parser/BNFFileReader.h"
#include "parser/Grammar.h"
#include "parser/ParseExtensions.h"
#include "parser/TokenStream.h"
#include "util/LogManager.h"
#include "driver/Configuration.h"
#include "driver/CompilerComponentsFactory.h"
#include "parser/SyntaxTreeBuilder.h"
#include "scanner/Token.h"

#include "ResourceHelpers.h"

#include <memory>
#include <vector>

using namespace testing;
using namespace parser;

namespace {

// Dummy initializer subparse: int __gnu_x = <tokens...>
// Used to exercise LrStop policies against the product table.
struct SubparseFixture {
    BNFFileReader reader;
    Grammar grammar { reader.readGrammar(getResourcePath("configuration/grammar.bnf")) };
    FilePersistedParsingTable table { getResourcePath("configuration/parsing_table"), &grammar };
    scanner::LexicalSession session;
    translation_unit::Context ctx { "t", 1 };

    scanner::Token tok(const std::string& id, const std::string& lexeme = {}) {
        return scanner::Token { id, lexeme.empty() ? id : lexeme, ctx };
    }

    // Token is not assignable (const members); build via push_back only.
    std::vector<scanner::Token> makeStreamTokens(
            std::initializer_list<std::pair<const char*, const char*>> rhs) {
        std::vector<scanner::Token> tokens;
        tokens.reserve(3 + rhs.size());
        tokens.push_back(tok("int"));
        tokens.push_back(tok("id", "__gnu_x"));
        tokens.push_back(tok("="));
        for (const auto& part : rhs) {
            tokens.push_back(tok(part.first, part.second ? part.second : part.first));
        }
        return tokens;
    }

    // Prefix `int __gnu_x =` is not live (matches GnuExtensions subparse).
    TokenStream makeStream(std::vector<scanner::Token>& tokens, std::size_t& index, bool& live) {
        index = 0;
        live = false;
        return TokenStream { [&]() {
            if (index >= tokens.size()) {
                live = true;
                return scanner::Token { scanner::Token::END, scanner::Token::END, ctx };
            }
            // First three tokens are the dummy declaration prefix.
            live = index >= 3;
            return tokens[index++];
        }, session };
    }
};

TEST(LR1Parser, parsesTestProgram) {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());
    configuration.setGrammarPath("resources/configuration/grammar.bnf");

    CompilerComponentsFactory compilerComponentsFactory { configuration };
    //LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });

    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));
    FilePersistedParsingTable parsingTable { getResourcePath("configuration/parsing_table"), &grammar };

    LR1Parser parser { parsingTable };
    scanner::LexicalSession session;
    auto builder = compilerComponentsFactory.makeSyntaxTreeBuilder(&grammar, session);
    ASSERT_NO_THROW(
            parser.parse(*compilerComponentsFactory.makeScannerForSourceFile(
                    getTestResourcePath("programs/example_prog.c"), session),
                    *builder));
}

// Complete stop must not consume `z = 5` as one cast_exp; stop before `=`.
TEST(LR1Parser, completeStopBeforeAssignmentOperator) {
    SubparseFixture fix;
    const int cast = fix.grammar.symbolId("<cast_exp>");
    auto tokens = fix.makeStreamTokens({
            { "id", "z" },
            { "=", nullptr },
            { "int_const", "5" },
            { ";", nullptr },
    });
    std::size_t index = 0;
    bool live = false;
    TokenStream stream = fix.makeStream(tokens, index, live);
    ast::AbstractSyntaxTreeBuilder builder { &fix.grammar, fix.session };

    ASSERT_EQ(runLrParse(fix.table, stream, builder, nullptr, LrStop::untilComplete(cast, &live)),
            LrFinish::Stopped);
    ASSERT_FALSE(builder.hasError());
    EXPECT_EQ(stream.getCurrentToken().id, "=");

    auto expr = builder.takeExpression();
    ASSERT_NE(expr, nullptr);
    auto* id = dynamic_cast<ast::IdentifierExpression*>(expr.get());
    ASSERT_NE(id, nullptr);
    EXPECT_EQ(id->getIdentifier(), "z");
}

// Outer cast (int)x must finish before stopping on `+`.
TEST(LR1Parser, completeStopAfterOuterCastNotInnerOperand) {
    SubparseFixture fix;
    const int cast = fix.grammar.symbolId("<cast_exp>");
    auto tokens = fix.makeStreamTokens({
            { "(", nullptr },
            { "int", nullptr },
            { ")", nullptr },
            { "id", "x" },
            { "+", nullptr },
            { "int_const", "1" },
            { ";", nullptr },
    });
    std::size_t index = 0;
    bool live = false;
    TokenStream stream = fix.makeStream(tokens, index, live);
    ast::AbstractSyntaxTreeBuilder builder { &fix.grammar, fix.session };

    ASSERT_EQ(runLrParse(fix.table, stream, builder, nullptr, LrStop::untilComplete(cast, &live)),
            LrFinish::Stopped);
    ASSERT_FALSE(builder.hasError());
    EXPECT_EQ(stream.getCurrentToken().id, "+");

    auto expr = builder.takeExpression();
    ASSERT_NE(expr, nullptr);
    EXPECT_NE(dynamic_cast<ast::TypeCast*>(expr.get()), nullptr);
}

// Classic lookahead stop still reduces assignment_exp looking at `,`.
TEST(LR1Parser, lookaheadStopOnCommaAfterAssignment) {
    SubparseFixture fix;
    const int assignment = fix.grammar.symbolId("<assignment_exp>");
    auto tokens = fix.makeStreamTokens({
            { "id", "z" },
            { "=", nullptr },
            { "int_const", "5" },
            { ",", nullptr },
            { "int_const", "9" },
    });
    std::size_t index = 0;
    bool live = false;
    TokenStream stream = fix.makeStream(tokens, index, live);
    ast::AbstractSyntaxTreeBuilder builder { &fix.grammar, fix.session };

    ASSERT_EQ(runLrParse(fix.table, stream, builder, nullptr,
                    LrStop::untilLookahead(assignment, ",", &live)),
            LrFinish::Stopped);
    ASSERT_FALSE(builder.hasError());
    EXPECT_EQ(stream.getCurrentToken().id, ",");
    ASSERT_NE(builder.takeExpression(), nullptr);
}

}
