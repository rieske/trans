#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/FiniteAutomaton.h"

#include "scanner/State.h"

using namespace testing;
using namespace scanner;
using std::map;
using std::string;

TEST(FiniteAutomaton, returnsEmptyLexemeByDefault) {
	FiniteAutomaton finiteAutomaton { nullptr, {}, {}};

	ASSERT_THAT(finiteAutomaton.getAccumulatedLexeme(), Eq(""));
}

TEST(FiniteAutomaton, accumulatesTokenBasedOnStateTransitions) {
	State startState("start", "");
	State accumulatingState("accumulating", "123");
	State finalState("final", "");
	startState.addTransition("", &startState);
	startState.addTransition("!", &accumulatingState);
	accumulatingState.addTransition("=", &accumulatingState);
	accumulatingState.addTransition("", &finalState);
	FiniteAutomaton finiteAutomaton { &startState, {}, {}};

	finiteAutomaton.updateState('!');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('=');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('a');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(true));

	ASSERT_THAT(finiteAutomaton.getAccumulatedLexeme(), Eq("!="));
}

TEST(FiniteAutomaton, ignoresTokensWithoutId) {
	State startState("start", "");
	State accumulatingState("accumulating", "");
	State finalState("final", "");
	startState.addTransition("", &startState);
	startState.addTransition("!", &accumulatingState);
	accumulatingState.addTransition("=", &accumulatingState);
	accumulatingState.addTransition("", &finalState);
	FiniteAutomaton finiteAutomaton { &startState, {}, {} };

	finiteAutomaton.updateState('!');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('=');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('a');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));

	ASSERT_THAT(finiteAutomaton.getAccumulatedLexeme(), Eq(""));
}

TEST(FiniteAutomaton, accumulatesIdentifierToken) {
	State startState("start", "");
	IdentifierState accumulatingState("accumulating", "123");
	State finalState("final", "");
	startState.addTransition("", &startState);
	startState.addTransition("v", &accumulatingState);
	accumulatingState.addTransition("oid", &accumulatingState);
	accumulatingState.addTransition("", &finalState);
	FiniteAutomaton finiteAutomaton { &startState, {}, {} };

	finiteAutomaton.updateState('v');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('o');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('i');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('d');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState(' ');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(true));

	ASSERT_THAT(finiteAutomaton.getAccumulatedLexeme(), Eq("void"));
}

TEST(FiniteAutomaton, looksUpKeywordIdentifier) {
	State startState("start", "");
	IdentifierState accumulatingState("accumulating", "123");
	State finalState("final", "");
	startState.addTransition("", &startState);
	startState.addTransition("v", &accumulatingState);
	accumulatingState.addTransition("oid", &accumulatingState);
	accumulatingState.addTransition("", &finalState);
	map<string, int> keywordIds;
	keywordIds["void"] = 999;
	FiniteAutomaton finiteAutomaton { &startState, keywordIds, {} };

	finiteAutomaton.updateState('v');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('o');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('i');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('d');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState(' ');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(true));

	ASSERT_THAT(finiteAutomaton.getAccumulatedLexeme(), Eq("void"));
}

TEST(FiniteAutomaton, returnsAdjacentTokens) {
	State startState("start", "");
	State operatorState("operator", "123");
	State finalState("final", "");
	startState.addTransition("", &startState);
	startState.addTransition("!", &operatorState);
	operatorState.addTransition("=", &operatorState);
	operatorState.addTransition("", &finalState);
	State identifierState("identifier", "234");
	startState.addTransition("a", &identifierState);
	identifierState.addTransition("", &finalState);
	FiniteAutomaton finiteAutomaton { &startState, {}, {} };

	finiteAutomaton.updateState('!');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('=');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('a');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(true));

	ASSERT_THAT(finiteAutomaton.getAccumulatedLexeme(), Eq("!="));

	finiteAutomaton.updateState('b');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(true));

	ASSERT_THAT(finiteAutomaton.getAccumulatedLexeme(), Eq("a"));
}
