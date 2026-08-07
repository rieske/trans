#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TokenMatcher.h"

#include <memory>

#include "parser/TokenStream.h"
#include "scanner/TypedefRegistry.h"
#include "scanner/Scanner.h"
#include "scanner/Token.h"
#include "types/Type.h"
#include "translation_unit/Context.h"

using namespace testing;
using namespace parser;
using namespace scanner;

TEST(TokenStream, usesScannerToRetrieveNextToken) {
    std::vector<scanner::Token> tokens { {"id", "variable", {"fileName", 10}}, {"add_op", "+", {"fileName", 50}} };
    int currentToken { 0 };
    scanner::TypedefRegistry typedefs;
    TokenStream tokenStream { [&]() { return tokens[currentToken++]; }, typedefs };

    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "id", "variable", { "fileName", 10 } }));
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));

    ASSERT_THAT(tokenStream.nextToken(), tokenMatches(Token { "add_op", "+", { "fileName", 50 } }));
    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "add_op", "+", { "fileName", 50 } }));
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));
}

TEST(TokenStream, insertsForgedTokenIntoStream) {
    std::vector<scanner::Token> tokens { {"id", "variable", {"fileName", 10}}, {"add_op", "+", {"fileName", 50}} };
    int currentToken { 0 };
    scanner::TypedefRegistry typedefs;
    TokenStream tokenStream { [&]() { return tokens[currentToken++]; }, typedefs };

    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "id", "variable", { "fileName", 10 } }));
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));

    tokenStream.forgeToken(Token { "forge", "forge", { "fileName", 99 } });
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(true));
    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "forge", "forge", { "fileName", 99 } }));
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(true));
    ASSERT_THAT(tokenStream.nextToken(), tokenMatches(Token { "id", "variable", { "fileName", 10 } }));
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));
    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "id", "variable", { "fileName", 10 } }));

    ASSERT_THAT(tokenStream.nextToken(), tokenMatches(Token { "add_op", "+", { "fileName", 50 } }));
    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "add_op", "+", { "fileName", 50 } }));
}

TEST(TokenStream, reclassifiesTypedefNameInExpressionContext) {
    // After return, a typedef spelling is an identifier (expression).
    scanner::TypedefRegistry typedefs;
    typedefs.add("size_t", type::unsignedLong());
    std::vector<scanner::Token> tokens {
        {"return", "return", {"f", 1}},
        {"typedef_name", "size_t", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, typedefs };
    ASSERT_EQ(ts.getCurrentToken().id, "return");
    ASSERT_EQ(ts.nextToken().id, "id");
    ASSERT_EQ(ts.getCurrentToken().lexeme, "size_t");
}

TEST(TokenStream, keepsTypedefNameInTypePosition) {
    scanner::TypedefRegistry typedefs;
    typedefs.add("size_t", type::unsignedLong());
    std::vector<scanner::Token> tokens {
        {"typedef_name", "size_t", {"f", 1}},
        {"id", "x", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, typedefs };
    ASSERT_EQ(ts.getCurrentToken().id, "typedef_name");
    ASSERT_EQ(ts.nextToken().id, "id"); // after typedef_name -> AsIdentifier
}
