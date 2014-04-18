#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <stdexcept>

#include "scanner/State.h"

using namespace testing;
using std::string;

TEST(State, isConstructedFromDefinitionRecord) {
	string definitionRecord = "%stateName 123";
	State state { definitionRecord, 0 };

	ASSERT_THAT(state.getTokenId(), Eq(123));
	ASSERT_THAT(state.getName(), Eq("stateName"));
}

TEST(State, doesNotRequireStateTypeModifier) {
	string definitionRecord = "stateName 123";
	State state { definitionRecord, 0 };

	ASSERT_THAT(state.getTokenId(), Eq(123));
	ASSERT_THAT(state.getName(), Eq("stateName"));
}

TEST(State, doesNotRequireTokenIdToBeSpecified) {
	string definitionRecord = "%stateName";
	State state { definitionRecord, 0 };

	ASSERT_THAT(state.getTokenId(), Eq(0));
	ASSERT_THAT(state.getName(), Eq("stateName"));
}

TEST(State, ignoresGarbageAfterStateName) {
	string definitionRecord = "%stateName asdf";
	State state { definitionRecord, 0 };

	ASSERT_THAT(state.getTokenId(), Eq(0));
	ASSERT_THAT(state.getName(), Eq("stateName"));
}

TEST(State, ignoresGarbageAfterTokenId) {
	string definitionRecord = "%stateName 123asdf";
	State state { definitionRecord, 0 };

	ASSERT_THAT(state.getTokenId(), Eq(123));
	ASSERT_THAT(state.getName(), Eq("stateName"));
}

TEST(State, throwsInvalidArgumentForEmptyRecord) {
	string definitionRecord;
	ASSERT_THROW(State state (definitionRecord, 0), std::invalid_argument);
}

TEST(State, constructsStateTransitionsFromStringOfCharacters) {
	string startStateDefinitionRecord = "startState";
	State startState { startStateDefinitionRecord, 0 };

	string transitionStateDefinitionRecord = "%transitionState 456";
	std::shared_ptr<State> transitionState(new State { transitionStateDefinitionRecord, 1 });

	startState.addTransition("abcd123", transitionState);

	ASSERT_THAT(startState.nextStateForCharacter('a'), Eq(transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('b'), Eq(transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('c'), Eq(transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('d'), Eq(transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('1'), Eq(transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('2'), Eq(transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('3'), Eq(transitionState));
	ASSERT_THROW(startState.nextStateForCharacter('-'), std::invalid_argument);
}
