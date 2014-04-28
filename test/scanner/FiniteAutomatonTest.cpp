#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/FiniteAutomaton.h"

#include "scanner/State.h"

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

	Token token = finiteAutomaton.getCurrentToken();

	ASSERT_THAT(token.type, Eq(-1));
	ASSERT_THAT(token.value, IsEmpty());
}

TEST(FiniteAutomaton, accumulatesTokenBasedOnStateTransitions) {
	shared_ptr<State> startState { new State { "start", 0 } };
	shared_ptr<State> accumulatingState { new State { "accumulating", 123 } };
	shared_ptr<State> finalState { new State { "final", 0 } };
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
	Token token = finiteAutomaton.getCurrentToken();

	ASSERT_THAT(token.type, Eq(123));
	ASSERT_THAT(token.value, Eq("!="));
}
