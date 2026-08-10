#include "parser/FilePersistedParsingTable.h"
#include "parser/BNFFileReader.h"
#include "parser/Action.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ResourceHelpers.h"
#include "scanner/Token.h"

using namespace parser;

TEST(FilePersistedParsingTable, readsTheParsingTable) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));

    FilePersistedParsingTable table(getResourcePath("configuration/parsing_table"), &grammar);

    // Smoke: every terminal has an action cell in the start state.
    for (const int terminalId : grammar.getTerminalIDs()) {
        scanner::Token token { grammar.getSymbolById(terminalId), "", { "", 0 } };
        ASSERT_NO_THROW(table.action(0, token));
    }
}

TEST(FilePersistedParsingTable, unknownLookaheadIsErrorActionNotThrow) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));
    FilePersistedParsingTable table(getResourcePath("configuration/parsing_table"), &grammar);

    scanner::Token token { "_Generic", "_Generic", { "t.c", 1 } };
    Action action;
    ASSERT_NO_THROW(action = table.action(0, token));
    EXPECT_EQ(action.kind(), Action::Kind::Error);
}

TEST(FilePersistedParsingTable, throwsRuntimeErrorForNonexistentParsingTableFile) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));

    ASSERT_THROW(FilePersistedParsingTable("parsingTableThatDoesNotExist", &grammar),
            std::runtime_error);
}
