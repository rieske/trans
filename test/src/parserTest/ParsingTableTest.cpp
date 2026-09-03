#include "parser/CompileParsingTable.h"
#include "parser/GrammarBuilder.h"
#include "parser/LookaheadActionTable.h"
#include "parser/ParsingTable.h"
#include "parser/SyntaxTreeBuilder.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/Token.h"
#include "util/Diagnostic.h"

#include <sstream>
#include <stdexcept>

namespace {

using namespace parser;
using testing::Eq;
using testing::HasSubstr;

Grammar tinyGrammar() {
    GrammarBuilder builder;
    builder.defineRule("<S>", { "a" });
    return builder.build();
}

ParsingTable emptyTable(const Grammar& grammar) {
    LookaheadActionTable actions;
    actions.setStateCount(1);
    return compileParsingTable(actions, {}, grammar);
}

scanner::Token tokenFor(const Grammar& grammar, int symbolId) {
    return { grammar.getSymbolById(symbolId), grammar.getSymbolById(symbolId), { "t.c", 1 }, symbolId };
}

class NullSyntaxTreeBuilder: public SyntaxTreeBuilder {
public:
    void makeTerminalNode(std::string, std::string, const translation_unit::Context&) override {}
    void makeNonterminalNode(const Production&) override {}
};

struct ErrorReport {
    NullSyntaxTreeBuilder treeBuilder;
    std::ostringstream logged;
    diag::Sink sink { logged };

    ErrorReport() {
        treeBuilder.setSink(&sink);
    }
};

TEST(ParsingTable, missingExplicitCellUsesStoredErrorCandidates) {
    Grammar grammar = tinyGrammar();
    LookaheadActionTable actions;
    actions.setStateCount(1);
    actions.setErrorCandidates(0, { grammar.getTerminalIDs().front() });
    ParsingTable table = compileParsingTable(actions, {}, grammar);

    const Action err = table.action(0, tokenFor(grammar, grammar.getEndSymbol()));
    EXPECT_THAT(err.kind(), Eq(Action::Kind::Error));
    EXPECT_THAT(err.toString(), Eq("e 0 " + std::to_string(grammar.getTerminalIDs().front())));
}

TEST(ParsingTable, missingExplicitCellWithoutCandidatesIsBareError) {
    Grammar grammar = tinyGrammar();
    ParsingTable table = emptyTable(grammar);

    const Action err = table.action(0, tokenFor(grammar, grammar.getEndSymbol()));
    EXPECT_THAT(err.kind(), Eq(Action::Kind::Error));
    EXPECT_THAT(err.toString(), Eq("e 0"));
}

TEST(ParsingTable, unknownLookaheadIsError) {
    Grammar grammar = tinyGrammar();
    ParsingTable table = emptyTable(grammar);

    const scanner::Token unknown { "_Generic", "_Generic", { "t.c", 1 }, 99999 };
    const Action err = table.action(0, unknown);
    EXPECT_THAT(err.kind(), Eq(Action::Kind::Error));
    EXPECT_THAT(err.toString(), HasSubstr("e "));
}

TEST(ParsingTable, unstampedLookaheadIsProgrammingError) {
    Grammar grammar = tinyGrammar();
    ParsingTable table = emptyTable(grammar);

    EXPECT_THROW(table.action(0, { "a", "a", { "t.c", 1 } }), std::logic_error);
}

TEST(ParsingTable, reportErrorListsExpectedTerminals) {
    Grammar grammar = tinyGrammar();
    const int expected = grammar.getTerminalIDs().front();
    LookaheadActionTable actions;
    actions.setStateCount(1);
    actions.setErrorCandidates(0, { expected });
    ParsingTable table = compileParsingTable(actions, {}, grammar);

    ErrorReport report;
    table.reportError(0, { "x", "x", { "t.c", 1 }, grammar.getEndSymbol() }, report.treeBuilder);

    EXPECT_TRUE(report.treeBuilder.hasError());
    EXPECT_TRUE(report.sink.hasErrors());
    EXPECT_THAT(report.logged.str(), HasSubstr("t.c:1: error: unexpected token: x expected:"));
    EXPECT_THAT(report.logged.str(), HasSubstr(grammar.getSymbolById(expected)));
}

TEST(ParsingTable, reportErrorWithNoCandidates) {
    Grammar grammar = tinyGrammar();
    ParsingTable table = emptyTable(grammar);

    ErrorReport report;
    table.reportError(0, { "x", "x", { "t.c", 1 }, grammar.getEndSymbol() }, report.treeBuilder);

    EXPECT_TRUE(report.treeBuilder.hasError());
    EXPECT_TRUE(report.sink.hasErrors());
    EXPECT_THAT(report.logged.str(), HasSubstr("t.c:1: error: unexpected token: x expected:"));
}

} // namespace
