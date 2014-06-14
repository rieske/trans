#include "parser/FilePersistedParsingTable.cpp"
#include "parser/BNFReader.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

TEST(FilePersistedParsingTable, readsTheParsingTable) {
	BNFReader bnfReader { "resources/configuration/grammar.bnf" };
	Grammar grammar = bnfReader.getGrammar();
	constexpr int stateCount = 809;

	FilePersistedParsingTable table("resources/configuration/parsing_table", grammar);

	for (parse_state state = 0; state < stateCount; ++state) {
		for (auto& terminal : grammar.terminals) {
			ASSERT_NO_THROW(table.action(state, terminal->getName()));
		}
	}
}

TEST(FilePersistedParsingTable, throwsRuntimeErrorForNonexistentParsingTableFile) {
	BNFReader bnfReader { "resources/configuration/grammar.bnf" };
	Grammar grammar = bnfReader.getGrammar();

	ASSERT_THROW(FilePersistedParsingTable("parsingTableThatDoesNotExist", grammar), std::runtime_error);
}
