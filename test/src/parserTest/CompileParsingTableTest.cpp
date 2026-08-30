#include "parser/CompileParsingTable.h"
#include "parser/GrammarBuilder.h"
#include "parser/LookaheadActionTable.h"
#include "parser/ParsingTable.h"
#include "scanner/Token.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ResourceHelpers.h"

#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <unordered_map>

namespace {

using namespace parser;
using testing::Eq;
using testing::HasSubstr;

Grammar tinyGrammar() {
    GrammarBuilder builder;
    builder.defineRule("<S>", { "a" });
    return builder.build();
}

TEST(CompileParsingTable, emptyErrorCandidatesStayEmpty) {
    Grammar grammar = tinyGrammar();
    LookaheadActionTable actions;
    actions.addAction(0, grammar.getEndSymbol(), Action::accept());
    ParsingTable table = compileParsingTable(actions, {}, grammar);

    scanner::Token end { grammar.getSymbolById(grammar.getEndSymbol()),
            grammar.getSymbolById(grammar.getEndSymbol()), { "t.c", 1 }, grammar.getEndSymbol() };
    EXPECT_EQ(table.action(0, end).kind(), Action::Kind::Accept);

    scanner::Token other { "a", "a", { "t.c", 1 }, grammar.getTerminalIDs().front() };
    EXPECT_THAT(table.action(0, other).toString(), Eq("e 0"));
}

TEST(CompileParsingTable, writeOmitsDummyErrorCandidate) {
    Grammar grammar = tinyGrammar();
    LookaheadActionTable actions;
    actions.addAction(0, grammar.getEndSymbol(), Action::accept());
    ParsingTable table = compileParsingTable(actions, {}, grammar);

    const std::string path = getTestResourcePath("programs/tmp/empty_errors_table.cpp");
    ScopedTempFile sourceFile { path };
    writeParsingTableSource(table, path);

    std::ifstream sourceStream { path };
    const std::string source { std::istreambuf_iterator<char>(sourceStream),
            std::istreambuf_iterator<char>() };
    EXPECT_THAT(source, HasSubstr("errorCandidates = nullptr"));
}

TEST(CompileParsingTable, rejectsShiftStateThatDoesNotFitPayload) {
    Grammar grammar = tinyGrammar();
    LookaheadActionTable actions;
    actions.addAction(0, grammar.getEndSymbol(),
            Action::shift(static_cast<parse_state>(std::numeric_limits<uint16_t>::max()) + 1));
    EXPECT_THROW(compileParsingTable(actions, {}, grammar), std::runtime_error);
}

TEST(CompileParsingTable, rejectsGotoThatDoesNotFitCell) {
    Grammar grammar = tinyGrammar();
    LookaheadActionTable actions;
    actions.addAction(0, grammar.getEndSymbol(), Action::accept());
    std::unordered_map<StateSymbolKey, parse_state, StateSymbolHash> gotos;
    gotos[{ 0, grammar.getNonterminalIDs().front() }] =
            static_cast<parse_state>(std::numeric_limits<int16_t>::max()) + 1;
    EXPECT_THROW(compileParsingTable(actions, gotos, grammar), std::runtime_error);
}

} // namespace
