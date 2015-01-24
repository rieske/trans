#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/LR1Item.h"
#include "parser/GrammarSymbol.h"
#include "parser/Production.h"

#include <sstream>
#include <stdexcept>

using namespace testing;
using namespace parser;

using std::unique_ptr;

TEST(LR1Item, constructsItemFromGrammarRuleAndLookahead) {
    unique_ptr<GrammarSymbol> nonterm { new GrammarSymbol("<nonterm>") };
    unique_ptr<GrammarSymbol> terminal1 { new GrammarSymbol("terminal1") };
    unique_ptr<GrammarSymbol> nonterm1 { new GrammarSymbol("<nonterm1>") };
    unique_ptr<GrammarSymbol> terminal2 { new GrammarSymbol("terminal2") };
    Production production { { terminal1.get(), nonterm1.get(), terminal2.get() }, 0 };
    unique_ptr<GrammarSymbol> lookahead { new GrammarSymbol("lookahead") };
    LR1Item item { nonterm.get(), production, { lookahead.get() } };

    ASSERT_THAT(item.getDefiningSymbol(), Eq(nonterm.get()));
    ASSERT_THAT(item.getVisited(), SizeIs(0));
    ASSERT_THAT(item.getExpectedSymbols(), SizeIs(3));
    ASSERT_THAT(item.getLookaheads(), Contains(lookahead.get()));
}

TEST(LR1Item, advancesTheVisitedSymbols) {
    unique_ptr<GrammarSymbol> nonterm { new GrammarSymbol("<nonterm>") };
    unique_ptr<GrammarSymbol> terminal1 { new GrammarSymbol("terminal1") };
    unique_ptr<GrammarSymbol> nonterm1 { new GrammarSymbol("<nonterm1>") };
    unique_ptr<GrammarSymbol> terminal2 { new GrammarSymbol("terminal2") };
    Production production { { terminal1.get(), nonterm1.get(), terminal2.get() }, 0 };
    unique_ptr<GrammarSymbol> lookahead { new GrammarSymbol("lookahead") };
    LR1Item item { nonterm.get(), production, { lookahead.get() } };

    LR1Item advanced1Item = item.advance();
    ASSERT_THAT(advanced1Item.getVisited(), SizeIs(1));
    ASSERT_THAT(advanced1Item.getExpectedSymbols(), SizeIs(2));

    LR1Item advanced2Item = advanced1Item.advance();
    ASSERT_THAT(advanced2Item.getVisited(), SizeIs(2));
    ASSERT_THAT(advanced2Item.getExpectedSymbols(), SizeIs(1));

    LR1Item advanced3Item = advanced2Item.advance();
    ASSERT_THAT(advanced3Item.getVisited(), SizeIs(3));
    ASSERT_THAT(advanced3Item.getExpectedSymbols(), SizeIs(0));
}

TEST(LR1Item, throwsAnExceptionIfAdvancedPastProductionBounds) {
    unique_ptr<GrammarSymbol> nonterm { new GrammarSymbol("<nonterm>") };
    unique_ptr<GrammarSymbol> terminal1 { new GrammarSymbol("terminal1") };
    Production production { { terminal1.get() }, 0 };
    std::shared_ptr<GrammarSymbol> lookahead = std::make_shared<GrammarSymbol>("lookahead");
    LR1Item item { nonterm.get(), production, { lookahead.get() } };

    ASSERT_THAT(item.advance().getVisited(), SizeIs(1));
    ASSERT_THAT(item.advance().getExpectedSymbols(), SizeIs(0));

    ASSERT_THROW(item.advance().advance(), std::out_of_range);
}

TEST(LR1Item, outputsTheItem) {
    unique_ptr<GrammarSymbol> nonterm { new GrammarSymbol("<nonterm>") };
    unique_ptr<GrammarSymbol> terminal1 { new GrammarSymbol("terminal1") };
    unique_ptr<GrammarSymbol> nonterm1 { new GrammarSymbol("<nonterm1>") };
    unique_ptr<GrammarSymbol> terminal2 { new GrammarSymbol("terminal2") };
    Production production { { terminal1.get(), nonterm1.get(), terminal2.get() }, 0 };
    unique_ptr<GrammarSymbol> lookahead { new GrammarSymbol("lookahead") };
    LR1Item item { nonterm.get(), production, { lookahead.get() } };

    std::stringstream sstream;
    sstream << item;
    ASSERT_THAT(sstream.str(), Eq("[ <nonterm> -> . terminal1 <nonterm1> terminal2 , lookahead ]\n"));

    sstream.str("");
    sstream << item.advance();
    ASSERT_THAT(sstream.str(), Eq("[ <nonterm> -> terminal1 . <nonterm1> terminal2 , lookahead ]\n"));

    sstream.str("");
    sstream << item.advance().advance();
    ASSERT_THAT(sstream.str(), Eq("[ <nonterm> -> terminal1 <nonterm1> . terminal2 , lookahead ]\n"));

    sstream.str("");
    sstream << item.advance().advance().advance();
    ASSERT_THAT(sstream.str(), Eq("[ <nonterm> -> terminal1 <nonterm1> terminal2 . , lookahead ]\n"));
}
