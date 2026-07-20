#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/Grammar.h"
#include "parser/GrammarBuilder.h"
#include "parser/BNFFileReader.h"
#include "parser/Production.h"

#include "ResourceHelpers.h"

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using namespace parser;
using testing::Eq;

TEST(Grammar, mapsTerminalBitsAndIds) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/expression_grammar.bnf"));
    const int id = grammar.getTerminalIDs().front();
    const auto bit = static_cast<std::size_t>(grammar.terminalBit(id));
    EXPECT_THAT(grammar.terminalIdFromBit(bit), Eq(id));
    EXPECT_TRUE(grammar.toLookaheadBits({ id }).test(bit));
}

TEST(Grammar, terminalBitThrowsForNonTerminalAndOutOfRange) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/expression_grammar.bnf"));
    EXPECT_THROW(grammar.terminalBit(grammar.getStartSymbol()), std::out_of_range);
    EXPECT_THROW(grammar.terminalBit(-2), std::out_of_range);
    EXPECT_THROW(grammar.terminalBit(10000), std::out_of_range);
    // Positive nonterminal id is in range of the mapping array but not a terminal.
    for (const int nonterminal : grammar.getNonterminalIDs()) {
        if (nonterminal >= 0) {
            EXPECT_THROW(grammar.terminalBit(nonterminal), std::out_of_range);
            break;
        }
    }
}

TEST(Grammar, skipsDuplicateTerminalIdsWhenBuildingBitMap) {
    std::map<std::string, int> symbolIDs { { "<S>", -1 }, { "a", 1 }, { "b", 2 } };
    // Duplicate terminal id 1 should be ignored by the bit map builder.
    std::vector<int> terminals { 1, 1, 2 };
    std::vector<int> nonterminals { -1 };
    std::vector<Production> rules { Production { -1, { 1 }, 0 } };
    Grammar grammar { symbolIDs, terminals, nonterminals, rules };
    EXPECT_THAT(grammar.terminalBit(1), Eq(0));
    EXPECT_THAT(grammar.terminalIdFromBit(0), Eq(1));
}

TEST(Grammar, rejectsTooManyTerminalsForLookaheadBitset) {
    std::map<std::string, int> symbolIDs;
    std::vector<int> terminals;
    std::vector<int> nonterminals { -1 };
    // Compact positive terminal ids 1..LOOKAHEAD_BITSET_SIZE+1 exceed the bitset capacity
    // (end symbol is appended as an extra terminal in the Grammar ctor).
    for (std::size_t i = 0; i < LOOKAHEAD_BITSET_SIZE + 1; ++i) {
        const int id = static_cast<int>(i + 1);
        symbolIDs["t" + std::to_string(id)] = id;
        terminals.push_back(id);
    }
    symbolIDs["<S>"] = -1;
    std::vector<Production> rules { Production { -1, { 1 }, 0 } };
    EXPECT_THROW(Grammar(symbolIDs, terminals, nonterminals, rules), std::runtime_error);
}

TEST(Grammar, rejectsTerminalIdOutsideMappingArray) {
    std::map<std::string, int> symbolIDs { { "<S>", -1 }, { "huge", 600 } };
    std::vector<int> terminals { 600 };
    std::vector<int> nonterminals { -1 };
    std::vector<Production> rules { Production { -1, { 600 }, 0 } };
    EXPECT_THROW(Grammar(symbolIDs, terminals, nonterminals, rules), std::runtime_error);
}

} // namespace
