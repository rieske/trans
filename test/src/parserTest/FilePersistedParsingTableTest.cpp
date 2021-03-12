#include "parser/FilePersistedParsingTable.h"
#include "parser/BNFFileReader.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ResourceHelpers.h"

using namespace parser;


TEST(FilePersistedParsingTable, readsTheParsingTable) {
    constexpr int stateCount = 809;
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));

    FilePersistedParsingTable table(getResourcePath("configuration/parsing_table"), &grammar);

    // FIXME:
    for (parse_state state = 0; state < stateCount; ++state) {
        /*for (auto& terminal : grammar.getTerminals()) {
            ASSERT_NO_THROW(table.action(state, terminal->getName()));
        }*/
    }
}

TEST(FilePersistedParsingTable, throwsRuntimeErrorForNonexistentParsingTableFile) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));

    ASSERT_THROW(FilePersistedParsingTable("parsingTableThatDoesNotExist", &grammar),
            std::runtime_error);
}
