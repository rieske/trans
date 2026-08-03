#include "parser/FilePersistedParsingTable.h"
#include "parser/BNFFileReader.h"
#include "parser/Action.h"
#include "parser/GeneratedParsingTable.h"
#include "parser/GrammarBuilder.h"
#include "parser/ParsingTableFile.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ResourceHelpers.h"
#include "TableAssertions.h"
#include "scanner/Token.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>

using namespace parser;

namespace {

Grammar expressionGrammar() {
    GrammarBuilder builder;
    builder.defineRule("<expr>", {"<term>", "+", "<expr>"});
    builder.defineRule("<expr>", {"<term>"});
    builder.defineRule("<term>", {"<factor>", "*", "<term>"});
    builder.defineRule("<term>", {"<factor>"});
    builder.defineRule("<factor>", {"(", "<expr>", ")"});
    builder.defineRule("<factor>", {"<operand>"});
    builder.defineRule("<operand>", {"identifier"});
    builder.defineRule("<operand>", {"constant"});
    return builder.build();
}

bool filesEqual(const std::string& leftPath, const std::string& rightPath) {
    std::ifstream left { leftPath };
    std::ifstream right { rightPath };
    return std::equal(std::istreambuf_iterator<char>(left), std::istreambuf_iterator<char>(),
            std::istreambuf_iterator<char>(right));
}

} // namespace

TEST(FilePersistedParsingTable, readsTheParsingTable) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));

    FilePersistedParsingTable table(getResourcePath("configuration/parsing_table"), &grammar);

    for (const int terminalId : grammar.getTerminalIDs()) {
        scanner::Token token { grammar.getSymbolById(terminalId), "", { "", 0 } };
        ASSERT_NO_THROW(table.action(0, token));
    }
}

TEST(FilePersistedParsingTable, unknownLookaheadIsErrorActionNotThrow) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));
    FilePersistedParsingTable table(getResourcePath("configuration/parsing_table"), &grammar);

    scanner::Token token { "__unknown_token__", "__unknown_token__", { "t.c", 1 } };
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

TEST(FilePersistedParsingTable, persistLoadRoundTripMatchesGeneratedActions) {
    Grammar grammar = expressionGrammar();
    GeneratedParsingTable generated { &grammar, AutomatonKind::LALR1 };

    ScopedTempFile persisted { "persist_roundtrip_table" };
    generated.persistToFile(persisted.path());
    FilePersistedParsingTable loaded { persisted.path(), &grammar };
    expectTablesMatch(generated, loaded);

    ScopedTempFile rePersisted { "persist_roundtrip_table_again" };
    loaded.persistToFile(rePersisted.path());
    EXPECT_TRUE(filesEqual(persisted.path(), rePersisted.path()));
}

TEST(FilePersistedParsingTable, persistUsesSparseFormatWithoutErrorCells) {
    Grammar grammar = expressionGrammar();
    GeneratedParsingTable generated { &grammar, AutomatonKind::LALR1 };
    ScopedTempFile persisted { "persist_sparse_format_table" };
    generated.persistToFile(persisted.path());

    std::ifstream in { persisted.path() };
    ParsingTableReader reader { in };
    EXPECT_EQ(reader.readHeader(), generated.stateCount());

    const auto actions = reader.readActions();
    EXPECT_FALSE(actions.empty());
    for (const auto& action : actions) {
        ASSERT_FALSE(action.serialized.empty());
        EXPECT_NE(action.serialized.front(), 'e') << action.serialized;
    }

    const auto errors = reader.readErrors();
    for (const auto& error : errors) {
        EXPECT_FALSE(error.candidates.empty());
    }
    EXPECT_FALSE(reader.readGotos().empty());
}

TEST(FilePersistedParsingTable, rejectsLegacyDenseTableHeader) {
    Grammar grammar = expressionGrammar();
    ScopedTempFile persisted { "persist_legacy_header" };
    {
        std::ofstream out { persisted.path() };
        out << "12\n%%\n";
    }
    EXPECT_THROW(FilePersistedParsingTable(persisted.path(), &grammar), std::runtime_error);
}
