#include "../parser/FilePersistedParsingTable.h"
#include "../parser/BNFFileGrammar.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ResourceHelpers.h"

using namespace parser;


TEST(FilePersistedParsingTable, readsTheParsingTable) {
	constexpr int stateCount = 809;

	FilePersistedParsingTable table(getResourcePath("configuration/parsing_table"), new BNFFileGrammar { getResourcePath("configuration/grammar.bnf") });

	// FIXME:
	for (parse_state state = 0; state < stateCount; ++state) {
		/*for (auto& terminal : grammar.getTerminals()) {
			ASSERT_NO_THROW(table.action(state, terminal->getName()));
		}*/
	}
}

TEST(FilePersistedParsingTable, throwsRuntimeErrorForNonexistentParsingTableFile) {
	ASSERT_THROW(FilePersistedParsingTable("parsingTableThatDoesNotExist", new BNFFileGrammar { getResourcePath("configuration/grammar.bnf") }),
			std::runtime_error);
}
