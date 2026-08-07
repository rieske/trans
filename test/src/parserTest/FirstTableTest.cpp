#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFFileReader.h"
#include "parser/Grammar.h"
#include "parser/FirstTable.h"

#include "ResourceHelpers.h"

#include <sstream>

using namespace testing;
using namespace parser;

// Product grammar: smoke that FIRST sets are non-empty and contain expected starters.
TEST(FirstTable, computesFirstTableForProductGrammar) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));

    FirstTable first { grammar };

    auto translationUnitFirst = first(grammar.symbolId("<translation_unit>"));
    EXPECT_THAT(translationUnitFirst, SizeIs(24));
    EXPECT_THAT(translationUnitFirst, IsSupersetOf({
                grammar.symbolId("int"),
                grammar.symbolId("char"),
                grammar.symbolId("void"),
                grammar.symbolId("float"),
                grammar.symbolId("double"),
                grammar.symbolId("struct"),
                grammar.symbolId("id"),
                grammar.symbolId("("),
                grammar.symbolId("*"),
                grammar.symbolId("__typeof__"),
            }));

    auto typeSpecFirst = first(grammar.symbolId("<type_spec>"));
    EXPECT_THAT(typeSpecFirst, UnorderedElementsAre(
                grammar.symbolId("void"),
                grammar.symbolId("char"),
                grammar.symbolId("short"),
                grammar.symbolId("int"),
                grammar.symbolId("long"),
                grammar.symbolId("float"),
                grammar.symbolId("double"),
                grammar.symbolId("signed"),
                grammar.symbolId("unsigned"),
                grammar.symbolId("struct"),
                grammar.symbolId("union"),
                grammar.symbolId("enum"),
                grammar.symbolId("typedef_name"),
                grammar.symbolId("__typeof__")));

    // Terminals are their own FIRST sets.
    EXPECT_THAT(first(grammar.symbolId("int")), ElementsAre(grammar.symbolId("int")));
    EXPECT_THAT(first(grammar.symbolId("id")), ElementsAre(grammar.symbolId("id")));
}

TEST(FirstTable, computesFirstTableForSimpleGrammarRules) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/expression_grammar.bnf"));

    FirstTable first { grammar };

    auto expressionFirst = first(grammar.symbolId("<expr>"));
    EXPECT_THAT(expressionFirst, SizeIs(3));
    EXPECT_THAT(expressionFirst, UnorderedElementsAre(grammar.symbolId("("), grammar.symbolId("constant"), grammar.symbolId("identifier")));

    auto termFirst = first(grammar.symbolId("<term>"));
    EXPECT_THAT(termFirst, SizeIs(3));
    EXPECT_THAT(termFirst, UnorderedElementsAre(grammar.symbolId("("), grammar.symbolId("constant"), grammar.symbolId("identifier")));

    auto factorFirst = first(grammar.symbolId("<factor>"));
    EXPECT_THAT(factorFirst, SizeIs(3));
    EXPECT_THAT(factorFirst, UnorderedElementsAre(grammar.symbolId("("), grammar.symbolId("constant"), grammar.symbolId("identifier")));

    auto operandFirst = first(grammar.symbolId("<operand>"));
    EXPECT_THAT(operandFirst, SizeIs(2));
    EXPECT_THAT(operandFirst, UnorderedElementsAre(grammar.symbolId("constant"), grammar.symbolId("identifier")));

    auto constantFirst = first(grammar.symbolId("constant"));
    EXPECT_THAT(constantFirst, SizeIs(1));
    EXPECT_THAT(constantFirst, ElementsAre(grammar.symbolId("constant")));

    auto identifierFirst = first(grammar.symbolId("identifier"));
    EXPECT_THAT(identifierFirst, SizeIs(1));
    EXPECT_THAT(identifierFirst, ElementsAre(grammar.symbolId("identifier")));

    std::stringstream expectedFirstStr;
    expectedFirstStr
        << "FIRST(<expr>): ( identifier constant \n"
        << "FIRST(<term>): ( identifier constant \n"
        << "FIRST(<factor>): ( identifier constant \n"
        << "FIRST(<operand>): identifier constant \n"
        << "FIRST(+): + \n"
        << "FIRST((): ( \n"
        << "FIRST()): ) \n"
        << "FIRST(identifier): identifier \n"
        << "FIRST(constant): constant \n"
        << "FIRST(*): * \n";
    EXPECT_THAT(first.str(grammar), Eq(expectedFirstStr.str()));
}
