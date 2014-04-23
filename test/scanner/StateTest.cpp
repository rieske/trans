#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <stdexcept>
#include <memory>

#include "scanner/State.h"

using namespace testing;
using std::string;
using std::shared_ptr;

TEST(State, isConstructedFromDefinitionRecord) {
	string definitionRecord = "%stateName 123";
	shared_ptr<State> state = State::createState(definitionRecord);

	ASSERT_THAT(state->getTokenId(), Eq(123));
	ASSERT_THAT(state->getName(), Eq("stateName"));
}

TEST(State, doesNotRequireStateTypeModifier) {
	string definitionRecord = "stateName 123";
	shared_ptr<State> state = State::createState(definitionRecord);

	ASSERT_THAT(state->getTokenId(), Eq(123));
	ASSERT_THAT(state->getName(), Eq("stateName"));
}

TEST(State, doesNotRequireTokenIdToBeSpecified) {
	string definitionRecord = "%stateName";
	shared_ptr<State> state = State::createState(definitionRecord);

	ASSERT_THAT(state->getTokenId(), Eq(0));
	ASSERT_THAT(state->getName(), Eq("stateName"));
}

TEST(State, ignoresGarbageAfterStateName) {
	string definitionRecord = "%stateName asdf";
	shared_ptr<State> state = State::createState(definitionRecord);

	ASSERT_THAT(state->getTokenId(), Eq(0));
	ASSERT_THAT(state->getName(), Eq("stateName"));
}

TEST(State, ignoresGarbageAfterTokenId) {
	string definitionRecord = "%stateName 123asdf";
	shared_ptr<State> state = State::createState(definitionRecord);

	ASSERT_THAT(state->getTokenId(), Eq(123));
	ASSERT_THAT(state->getName(), Eq("stateName"));
}

TEST(State, throwsInvalidArgumentForEmptyRecord) {
	string definitionRecord;
	ASSERT_THROW(State::createState(definitionRecord), std::invalid_argument);
}

TEST(State, constructsStateTransitionsFromStringOfCharacters) {
	string startStateDefinitionRecord = "startState";
	shared_ptr<State> startState = State::createState(startStateDefinitionRecord);

	string transitionStateDefinitionRecord = "%transitionState 456";
	std::shared_ptr<State> transitionState = State::createState(transitionStateDefinitionRecord);

	startState->addTransition("abcd123", transitionState);

	ASSERT_THAT(startState->nextStateForCharacter('a'), Eq(transitionState));
	ASSERT_THAT(startState->nextStateForCharacter('b'), Eq(transitionState));
	ASSERT_THAT(startState->nextStateForCharacter('c'), Eq(transitionState));
	ASSERT_THAT(startState->nextStateForCharacter('d'), Eq(transitionState));
	ASSERT_THAT(startState->nextStateForCharacter('1'), Eq(transitionState));
	ASSERT_THAT(startState->nextStateForCharacter('2'), Eq(transitionState));
	ASSERT_THAT(startState->nextStateForCharacter('3'), Eq(transitionState));
	ASSERT_THROW(startState->nextStateForCharacter('-'), std::invalid_argument);
}
