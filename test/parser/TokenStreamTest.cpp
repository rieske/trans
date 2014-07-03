#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "../scanner/TokenMatcher.h"

#include <memory>

#include "parser/TokenStream.h"
#include "scanner/Scanner.h"
#include "scanner/Token.h"

using namespace testing;
using std::unique_ptr;

class MockScanner: public Scanner {
public:
	MOCK_METHOD0(nextToken, Token());
};

TEST(TokenStream, usesScannerToRetrieveNextToken) {
	MockScanner scanner;
	{
		InSequence sequence;

		EXPECT_CALL(scanner, nextToken()).WillOnce(Return(Token { "id", "variable", 10 }));
		EXPECT_CALL(scanner, nextToken()).WillOnce(Return(Token { "add_op", "+", 50 }));
	}

	TokenStream tokenStream { &scanner };

	ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "id", "variable", 10 }));
	ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));

	ASSERT_THAT(tokenStream.nextToken(), tokenMatches(Token { "add_op", "+", 50 }));
	ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "add_op", "+", 50 }));
	ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));
}

TEST(TokenStream, insertsForgedTokenIntoStream) {
	MockScanner scanner;
	{
		InSequence sequence;

		EXPECT_CALL(scanner, nextToken()).WillOnce(Return(Token { "id", "variable", 10 }));
		EXPECT_CALL(scanner, nextToken()).WillOnce(Return(Token { "add_op", "+", 50 }));
	}

	TokenStream tokenStream { &scanner };

	ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "id", "variable", 10 }));
	ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));

	tokenStream.forgeToken(Token { "forge", "forge", 99 });
	ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(true));
	ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "forge", "forge", 99 }));
	ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(true));
	ASSERT_THAT(tokenStream.nextToken(), tokenMatches(Token { "id", "variable", 10 }));
	ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));
	ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "id", "variable", 10 }));

	ASSERT_THAT(tokenStream.nextToken(), tokenMatches(Token { "add_op", "+", 50 }));
	ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "add_op", "+", 50 }));
}
