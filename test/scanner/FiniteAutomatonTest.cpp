#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/FiniteAutomaton.h"

#include "scanner/State.h"
#include "scanner/IdentifierState.h"
#include "TokenMatcher.h"

#include <memory>
#include <map>

using namespace testing;
using std::map;
using std::shared_ptr;
using std::string;

TEST(FiniteAutomaton, returnsEofTokenByDefault) {
	shared_ptr<State> startState;
	shared_ptr<State> finalState;
	map<string, unsigned> keywordIds;
	FiniteAutomaton finiteAutomaton { startState, keywordIds };

	ASSERT_THAT(finiteAutomaton.getAccumulatedLexeme(), Eq(""));
}

TEST(FiniteAutomaton, accumulatesTokenBasedOnStateTransitions) {
	shared_ptr<State> startState { new State { "start", "" } };
	shared_ptr<State> accumulatingState { new State { "accumulating", "123" } };
	shared_ptr<State> finalState { new State { "final", "" } };
	startState->addTransition("", startState);
	startState->addTransition("!", accumulatingState);
	accumulatingState->addTransition("=", accumulatingState);
	accumulatingState->addTransition("", finalState);
	map<string, unsigned> keywordIds;
	FiniteAutomaton finiteAutomaton { startState, keywordIds };

	finiteAutomaton.updateState('!');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('=');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('a');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(true));

	ASSERT_THAT(finiteAutomaton.getAccumulatedLexeme(), Eq("!="));
}

TEST(FiniteAutomaton, ignoresTokensWithoutId) {
	shared_ptr<State> startState { new State { "start", "" } };
	shared_ptr<State> accumulatingState { new State { "accumulating", "" } };
	shared_ptr<State> finalState { new State { "final", "" } };
	startState->addTransition("", startState);
	startState->addTransition("!", accumulatingState);
	accumulatingState->addTransition("=", accumulatingState);
	accumulatingState->addTransition("", finalState);
	map<string, unsigned> keywordIds;
	FiniteAutomaton finiteAutomaton { startState, keywordIds };

	finiteAutomaton.updateState('!');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('=');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));
	finiteAutomaton.updateState('a');
	ASSERT_THAT(finiteAutomaton.isAtFinalState(), Eq(false));

	ASSERT_THAT(finiteAutomaton.getAccumulatedLexeme(), Eq(""));
}

TEST(FiniteAutomaton, accumulatesIdentifierToken) {
	shared_ptr<State> startState { new State { "start", "" } };
	shared_ptr<State> accumulatingState { new IdentifierState { "accumulating", "123" } };
	shared_ptr<State> finalState { new State { "final", "" } };
	startState->addTransition("", startState);
	startState->addTransition("v", accumulatingState);
	accumulatingState->addTransition("oid", accumulatingState);
	accumulatingState->addTransition("", finalState);
	map<string, unsigned> keywordIds;
	FiniteAutomaton finiteAutomaton { startState, keywordIds };

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
	shared_ptr<State> startState { new State { "start", "" } };
	shared_ptr<State> accumulatingState { new IdentifierState { "accumulating", "123" } };
	shared_ptr<State> finalState { new State { "final", "" } };
	startState->addTransition("", startState);
	startState->addTransition("v", accumulatingState);
	accumulatingState->addTransition("oid", accumulatingState);
	accumulatingState->addTransition("", finalState);
	map<string, unsigned> keywordIds;
	keywordIds["void"] = 999;
	FiniteAutomaton finiteAutomaton { startState, keywordIds };

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
	shared_ptr<State> startState { new State { "start", "" } };
	shared_ptr<State> operatorState { new State { "operator", "123" } };
	shared_ptr<State> finalState { new State { "final", "" } };
	startState->addTransition("", startState);
	startState->addTransition("!", operatorState);
	operatorState->addTransition("=", operatorState);
	operatorState->addTransition("", finalState);
	shared_ptr<State> identifierState { new State { "identifier", "234" } };
	startState->addTransition("a", identifierState);
	identifierState->addTransition("", finalState);
	map<string, unsigned> keywordIds;
	FiniteAutomaton finiteAutomaton { startState, keywordIds };

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
