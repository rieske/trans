#ifndef TABLE_ASSERTIONS_H_
#define TABLE_ASSERTIONS_H_

#include "gtest/gtest.h"

#include "parser/Grammar.h"
#include "parser/ParsingTable.h"
#include "scanner/Token.h"

inline void expectTablesMatch(const parser::ParsingTable& generated, const parser::ParsingTable& loaded) {
    ASSERT_EQ(loaded.stateCount(), generated.stateCount());
    ASSERT_NE(generated.getGrammar(), nullptr);
    const parser::Grammar& grammar = *generated.getGrammar();
    for (std::size_t state = 0; state < generated.stateCount(); ++state) {
        for (const int terminalId : grammar.getTerminalIDs()) {
            scanner::Token token { grammar.getSymbolById(terminalId), "", { "", 0 }, terminalId };
            EXPECT_EQ(loaded.action(state, token).serialize(),
                    generated.action(state, token).serialize())
                    << "state=" << state
                    << " terminal=" << grammar.getSymbolById(terminalId);
        }
        for (const int nonterminalId : grammar.getNonterminalIDs()) {
            EXPECT_EQ(loaded.tryGoTo(state, nonterminalId), generated.tryGoTo(state, nonterminalId))
                    << "state=" << state
                    << " nonterminal=" << grammar.getSymbolById(nonterminalId);
        }
        EXPECT_EQ(loaded.tryGoTo(state, grammar.getStartSymbol()),
                generated.tryGoTo(state, grammar.getStartSymbol()))
                << "state=" << state << " start symbol";
    }
}

#endif
