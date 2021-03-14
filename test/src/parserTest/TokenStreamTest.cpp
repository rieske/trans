#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TokenMatcher.h"

#include <memory>

#include "parser/TokenStream.h"
#include "scanner/Scanner.h"
#include "scanner/Token.h"
#include "translation_unit/Context.h"

using namespace testing;
using namespace parser;
using namespace scanner;

TEST(TokenStream, usesScannerToRetrieveNextToken) {
    std::vector<scanner::Token> tokens { {"id", "variable", {"fileName", 10}}, {"add_op", "+", {"fileName", 50}} };
    int currentToken { 0 };
    TokenStream tokenStream { [&]() { return tokens[currentToken++]; } };

    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "id", "variable", { "fileName", 10 } }));
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));

    ASSERT_THAT(tokenStream.nextToken(), tokenMatches(Token { "add_op", "+", { "fileName", 50 } }));
    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "add_op", "+", { "fileName", 50 } }));
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));
}

TEST(TokenStream, insertsForgedTokenIntoStream) {
    std::vector<scanner::Token> tokens { {"id", "variable", {"fileName", 10}}, {"add_op", "+", {"fileName", 50}} };
    int currentToken { 0 };
    TokenStream tokenStream { [&]() { return tokens[currentToken++]; } };

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
