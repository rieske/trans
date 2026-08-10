#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TokenMatcher.h"

#include "parser/TokenStream.h"
#include "scanner/LexicalSession.h"
#include "scanner/Token.h"
#include "types/Type.h"

using namespace testing;
using namespace parser;
using namespace scanner;

TEST(TokenStream, usesScannerToRetrieveNextToken) {
    std::vector<scanner::Token> tokens { {"id", "variable", {"fileName", 10}}, {"+", "+", {"fileName", 50}} };
    int currentToken { 0 };
    scanner::LexicalSession session;
    TokenStream tokenStream { [&]() { return tokens[currentToken++]; }, session };

    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "id", "variable", { "fileName", 10 } }));
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));

    ASSERT_THAT(tokenStream.nextToken(), tokenMatches(Token { "+", "+", { "fileName", 50 } }));
    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "+", "+", { "fileName", 50 } }));
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));
}

TEST(TokenStream, peekDoesNotConsume) {
    std::vector<scanner::Token> tokens {
        {"(", "(", {"f", 1}},
        {"{", "{", {"f", 1}},
        {"}", "}", {"f", 1}},
    };
    int currentToken { 0 };
    scanner::LexicalSession session;
    TokenStream tokenStream { [&]() { return tokens[currentToken++]; }, session };

    ASSERT_EQ(tokenStream.getCurrentToken().id, "(");
    ASSERT_EQ(tokenStream.peek().id, "{");
    ASSERT_EQ(tokenStream.getCurrentToken().id, "(");
    ASSERT_EQ(tokenStream.peek().id, "{");
    ASSERT_EQ(tokenStream.nextToken().id, "{");
    ASSERT_EQ(tokenStream.getCurrentToken().id, "{");
}

TEST(TokenStream, takeRawDoesNotEnterOrLeaveBlock) {
    scanner::LexicalSession session;
    session.typedefs.add("T", type::signedInteger());
    session.typedefs.pushIdentifierShadowScope();
    session.typedefs.addIdentifierShadow("T");
    std::vector<scanner::Token> tokens {
        {"{", "{", {"f", 1}},
        {"typedef_name", "T", {"f", 1}},
        {"}", "}", {"f", 1}},
        {"typedef_name", "T", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };

    ASSERT_EQ(ts.takeRaw().id, "{");
    ASSERT_EQ(ts.getCurrentToken().id, "id");
    ASSERT_EQ(ts.takeRaw().lexeme, "T");
    ASSERT_EQ(ts.takeRaw().id, "}");
    ASSERT_EQ(ts.getCurrentToken().id, "id");
}

TEST(TokenStream, ungetPutsTokenBackAsCurrent) {
    std::vector<scanner::Token> tokens {
        {"id", "ap", {"f", 1}},
        {",", ",", {"f", 1}},
        {"int", "int", {"f", 1}},
    };
    int i = 0;
    scanner::LexicalSession session;
    TokenStream ts { [&]() { return tokens[i++]; }, session };

    ASSERT_EQ(ts.takeRaw().id, "id");
    ASSERT_EQ(ts.getCurrentToken().id, ",");
    ASSERT_EQ(ts.takeRaw().id, ",");
    ASSERT_EQ(ts.getCurrentToken().id, "int");
    ts.unget(scanner::Token { ",", ",", {"f", 1} });
    ASSERT_EQ(ts.getCurrentToken().id, ",");
    ASSERT_EQ(ts.nextToken().id, "int");
    ASSERT_EQ(ts.getCurrentToken().id, "int");
}

TEST(TokenStream, takeRawConsumesPeekedLookahead) {
    std::vector<scanner::Token> tokens {
        {"(", "(", {"f", 1}},
        {"{", "{", {"f", 1}},
        {"}", "}", {"f", 1}},
    };
    int i = 0;
    scanner::LexicalSession session;
    TokenStream ts { [&]() { return tokens[i++]; }, session };

    ASSERT_EQ(ts.getCurrentToken().id, "(");
    ASSERT_EQ(ts.peek().id, "{");
    ASSERT_EQ(ts.takeRaw().id, "(");
    ASSERT_EQ(ts.getCurrentToken().id, "{");
}

TEST(TokenStream, insertsForgedTokenIntoStream) {
    std::vector<scanner::Token> tokens { {"id", "variable", {"fileName", 10}}, {"+", "+", {"fileName", 50}} };
    int currentToken { 0 };
    scanner::LexicalSession session;
    TokenStream tokenStream { [&]() { return tokens[currentToken++]; }, session };

    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "id", "variable", { "fileName", 10 } }));
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));

    tokenStream.forgeToken(Token { "forge", "forge", { "fileName", 99 } });
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(true));
    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "forge", "forge", { "fileName", 99 } }));
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(true));
    ASSERT_THAT(tokenStream.nextToken(), tokenMatches(Token { "id", "variable", { "fileName", 10 } }));
    ASSERT_THAT(tokenStream.currentTokenIsForged(), Eq(false));
    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "id", "variable", { "fileName", 10 } }));

    ASSERT_THAT(tokenStream.nextToken(), tokenMatches(Token { "+", "+", { "fileName", 50 } }));
    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(Token { "+", "+", { "fileName", 50 } }));
}

TEST(TokenStream, reclassifiesTypedefNameInExpressionContext) {
    // After return, a typedef spelling is an identifier (expression).
    scanner::LexicalSession session;
    session.typedefs.add("size_t", type::unsignedLong());
    std::vector<scanner::Token> tokens {
        {"return", "return", {"f", 1}},
        {"typedef_name", "size_t", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, "return");
    ASSERT_EQ(ts.nextToken().id, "id");
    ASSERT_EQ(ts.getCurrentToken().lexeme, "size_t");
}

TEST(TokenStream, keepsTypedefNameInTypePosition) {
    scanner::LexicalSession session;
    session.typedefs.add("size_t", type::unsignedLong());
    std::vector<scanner::Token> tokens {
        {"typedef_name", "size_t", {"f", 1}},
        {"id", "x", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, "typedef_name");
    ASSERT_EQ(ts.nextToken().id, "id"); // after typedef_name -> AsIdentifier
}

TEST(TokenStream, forcesIdWhenIdentifierShadow) {
    scanner::LexicalSession session;
    session.typedefs.add("T", type::signedInteger());
    session.typedefs.pushIdentifierShadowScope();
    session.typedefs.addIdentifierShadow("T");
    std::vector<scanner::Token> tokens {
        {"typedef_name", "T", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, "id");
}

TEST(TokenStream, tagAfterStructIsIdentifier) {
    scanner::LexicalSession session;
    session.typedefs.add("S", type::signedInteger());
    std::vector<scanner::Token> tokens {
        {"struct", "struct", {"f", 1}},
        {"typedef_name", "S", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, "struct");
    ASSERT_EQ(ts.nextToken().id, "id");
}

TEST(TokenStream, memberAfterDotIsIdentifier) {
    scanner::LexicalSession session;
    session.typedefs.add("T", type::signedInteger());
    std::vector<scanner::Token> tokens {
        {".", ".", {"f", 1}},
        {"typedef_name", "T", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, ".");
    ASSERT_EQ(ts.nextToken().id, "id");
}

TEST(TokenStream, declaratorAfterStarIsIdentifier) {
    scanner::LexicalSession session;
    session.typedefs.add("T", type::signedInteger());
    std::vector<scanner::Token> tokens {
        {"typedef_name", "T", {"f", 1}},
        {"*", "*", {"f", 1}},
        {"typedef_name", "T", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, "typedef_name");
    ASSERT_EQ(ts.nextToken().id, "*");
    ASSERT_EQ(ts.nextToken().id, "id");
}

TEST(TokenStream, braceScopePopsIdentifierShadow) {
    scanner::LexicalSession session;
    session.typedefs.add("T", type::signedInteger());
    std::vector<scanner::Token> tokens {
        {"{", "{", {"f", 1}},
        {"typedef_name", "T", {"f", 1}},
        {"}", "}", {"f", 1}},
        {"typedef_name", "T", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, "{");
    // nextToken('{') opens a shadow scope; add the shadow on that frame.
    ts.nextToken();
    session.typedefs.addIdentifierShadow("T");
    ASSERT_EQ(ts.getCurrentToken().id, "id"); // shadowed inside brace
    ASSERT_EQ(ts.nextToken().id, "}");
    ASSERT_EQ(ts.nextToken().id, "typedef_name"); // shadow popped
}

TEST(TokenStream, promotesIdToTypedefNameInTypePosition) {
    scanner::LexicalSession session;
    session.typedefs.add("T", type::signedInteger());
    std::vector<scanner::Token> tokens {
        {"id", "T", {"f", 1}},
        {"id", "x", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, "typedef_name");
}

TEST(TokenStream, constKeepsTypedefName) {
    // Qualifiers keep type context: const foo_t x keeps foo_t as typedef_name.
    scanner::LexicalSession session;
    session.typedefs.add("foo_t", type::signedInteger());
    std::vector<scanner::Token> tokens {
        {"const", "const", {"f", 1}},
        {"typedef_name", "foo_t", {"f", 1}},
        {"id", "x", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, "const");
    ASSERT_EQ(ts.nextToken().id, "typedef_name");
    ASSERT_EQ(ts.getCurrentToken().lexeme, "foo_t");
}

TEST(TokenStream, afterPrimitiveTypeSpecDeclaratorIsIdentifier) {
    // After int/char/..., the next spelling is a declarator name (`int T`), not a type.
    scanner::LexicalSession session;
    session.typedefs.add("T", type::signedInteger());
    std::vector<scanner::Token> tokens {
        {"int", "int", {"f", 1}},
        {"typedef_name", "T", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, "int");
    ASSERT_EQ(ts.nextToken().id, "id");
    ASSERT_EQ(ts.getCurrentToken().lexeme, "T");
}

TEST(TokenStream, afterConstThenPrimitiveDeclaratorIsIdentifier) {
    // const int T: qualifier keeps type, then int forces declarator context.
    scanner::LexicalSession session;
    session.typedefs.add("T", type::signedInteger());
    std::vector<scanner::Token> tokens {
        {"const", "const", {"f", 1}},
        {"int", "int", {"f", 1}},
        {"typedef_name", "T", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, "const");
    ASSERT_EQ(ts.nextToken().id, "int");
    ASSERT_EQ(ts.nextToken().id, "id");
    ASSERT_EQ(ts.getCurrentToken().lexeme, "T");
}

TEST(TokenStream, colonDoesNotForceTypeRestart) {
    // ':' is intentionally omitted from AsType restart (label/case vs ternary).
    // After a primary (AsIdentifier), keep context: typedef spelling stays id.
    scanner::LexicalSession session;
    session.typedefs.add("size_t", type::unsignedLong());
    std::vector<scanner::Token> tokens {
        {"return", "return", {"f", 1}},
        {"typedef_name", "size_t", {"f", 1}},
        {":", ":", {"f", 1}},
        {"typedef_name", "size_t", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, "return");
    ASSERT_EQ(ts.nextToken().id, "id"); // expression after return
    ASSERT_EQ(ts.nextToken().id, ":");
    // After ':', context kept (AsIdentifier) so next typedef spelling is id.
    ASSERT_EQ(ts.nextToken().id, "id");
}

TEST(TokenStream, commaRestartsTypePositionForTypedefName) {
    // Product: ',' is AsType restart so multi-param type lists work
    // (`void f(int a, size_t b)` keeps size_t as typedef_name after comma).
    scanner::LexicalSession session;
    session.typedefs.add("size_t", type::unsignedLong());
    std::vector<scanner::Token> tokens {
        {"int", "int", {"f", 1}},
        {"id", "a", {"f", 1}},
        {",", ",", {"f", 1}},
        {"typedef_name", "size_t", {"f", 1}},
        {"id", "b", {"f", 1}},
        {")", ")", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, "int");
    ASSERT_EQ(ts.nextToken().id, "id");
    ASSERT_EQ(ts.getCurrentToken().lexeme, "a");
    ASSERT_EQ(ts.nextToken().id, ",");
    ASSERT_EQ(ts.nextToken().id, "typedef_name");
    ASSERT_EQ(ts.getCurrentToken().lexeme, "size_t");
}

TEST(TokenStream, pendingParameterShadowFlushesOnBrace) {
    scanner::LexicalSession session;
    session.typedefs.add("T", type::signedInteger());
    session.typedefs.addPendingParameterShadow("T");
    std::vector<scanner::Token> tokens {
        {"{", "{", {"f", 1}},
        {"typedef_name", "T", {"f", 1}},
        {"}", "}", {"f", 1}},
        {"typedef_name", "T", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, "{");
    ts.nextToken(); // consume `{`, flush pending into new scope
    ASSERT_EQ(ts.getCurrentToken().id, "id"); // shadowed
    ASSERT_EQ(ts.nextToken().id, "}");
    ASSERT_EQ(ts.nextToken().id, "typedef_name"); // pop restores type
}

TEST(TokenStream, pendingParameterShadowClearedOnSemicolon) {
    scanner::LexicalSession session;
    session.typedefs.add("T", type::signedInteger());
    session.typedefs.addPendingParameterShadow("T");
    std::vector<scanner::Token> tokens {
        {";", ";", {"f", 1}},
        {"typedef_name", "T", {"f", 1}},
        {"id", "x", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, ";");
    ts.nextToken(); // clear pending
    ASSERT_EQ(ts.getCurrentToken().id, "typedef_name");
    ASSERT_EQ(ts.getCurrentToken().lexeme, "T");
}

TEST(TokenStream, braceScopePopsObjectType) {
    scanner::LexicalSession session;
    session.objects.add("x", type::signedInteger());
    std::vector<scanner::Token> tokens {
        {"{", "{", {"f", 1}},
        {"}", "}", {"f", 1}},
        {";", ";", {"f", 1}},
    };
    int i = 0;
    TokenStream ts { [&]() { return tokens[i++]; }, session };
    ASSERT_EQ(ts.getCurrentToken().id, "{");
    ts.nextToken();
    session.objects.add("x", type::signedCharacter());
    auto inner = session.objects.lookup("x");
    ASSERT_TRUE(inner.has_value());
    EXPECT_TRUE(inner->equivalentTo(type::signedCharacter()));
    ts.nextToken();
    auto outer = session.objects.lookup("x");
    ASSERT_TRUE(outer.has_value());
    EXPECT_TRUE(outer->equivalentTo(type::signedInteger()));
}

