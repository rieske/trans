#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TokenMatcher.h"

#include <memory>

#include "../parser/TokenStream.h"
#include "../scanner/Scanner.h"
#include "../scanner/Token.h"
#include "../translation_unit/Context.h"

using namespace testing;
using namespace parser;

using std::unique_ptr;

class MockScanner: public Scanner {
public:
    MOCK_METHOD0(nextToken, Token());
};

TEST(TokenStream, usesScannerToRetrieveNextToken) {
    MockScanner scanner;
    {
        InSequence sequence;

        EXPECT_CALL(scanner, nextToken()).WillOnce(Return(Token { "id", "variable", { "fileName", 10 } }));
        EXPECT_CALL(scanner, nextToken()).WillOnce(Return(Token { "add_op", "+", { "fileName", 50 } }));
    }

    TokenStream tokenStream { &scanner };

    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "id", "variable", { "fileName", 10 } }));
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));

    ASSERT_THAT(tokenStream.nextToken(), tokenMatches(Token { "add_op", "+", { "fileName", 50 } }));
    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "add_op", "+", { "fileName", 50 } }));
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));
}

TEST(TokenStream, insertsForgedTokenIntoStream) {
    MockScanner scanner;
    {
        InSequence sequence;

        EXPECT_CALL(scanner, nextToken()).WillOnce(Return(Token { "id", "variable", { "fileName", 10 } }));
        EXPECT_CALL(scanner, nextToken()).WillOnce(Return(Token { "add_op", "+", { "fileName", 50 } }));
    }

    TokenStream tokenStream { &scanner };

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
