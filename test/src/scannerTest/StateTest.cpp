#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/State.h"

using namespace testing;
using namespace scanner;

TEST(State, doesNotNeedKeywordLookup) {
	State state("stateName", "123");

	ASSERT_THAT(state.getTokenId(), Eq("123"));
	ASSERT_THAT(state.getName(), Eq("stateName"));
	ASSERT_THAT(state.needsKeywordLookup(), Eq(false));
}

TEST(State, doesNotRequireTokenIdToBeSpecified) {
	State state("stateName", "");

	ASSERT_THAT(state.getTokenId(), Eq(""));
	ASSERT_THAT(state.getName(), Eq("stateName"));
}

TEST(State, constructsStateTransitionsFromStringOfCharacters) {
	State startState("startState", "");

	State transitionState("transitionState", "456");

	startState.addTransition("abcd123", &transitionState);

	ASSERT_THAT(startState.nextStateForCharacter('a'), Eq(&transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('b'), Eq(&transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('c'), Eq(&transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('d'), Eq(&transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('1'), Eq(&transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('2'), Eq(&transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('3'), Eq(&transitionState));
	ASSERT_THROW(startState.nextStateForCharacter(' '), std::runtime_error);
	ASSERT_THROW(startState.nextStateForCharacter('\t'), std::runtime_error);
	ASSERT_THROW(startState.nextStateForCharacter('\n'), std::runtime_error);
	ASSERT_THROW(startState.nextStateForCharacter('-'), std::runtime_error);
}

TEST(State, constructsTransitionForAnyCharacter) {
	State startState("startState", "0");

	State transitionState("transitionState", "456");

	startState.addTransition("", &transitionState);

	ASSERT_THAT(startState.nextStateForCharacter('a'), Eq(&transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('b'), Eq(&transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('3'), Eq(&transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('-'), Eq(&transitionState));
	ASSERT_THAT(startState.nextStateForCharacter(' '), Eq(&transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('\n'), Eq(&transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('\t'), Eq(&transitionState));
	ASSERT_THAT(startState.nextStateForCharacter('\0'), Eq(&transitionState));
}

TEST(IdentifierState, needsLookup) {
	IdentifierState state("identifier", "123");

	ASSERT_THAT(state.getTokenId(), Eq("123"));
	ASSERT_THAT(state.getName(), Eq("identifier"));
	ASSERT_THAT(state.needsKeywordLookup(), Eq(true));
}

TEST(IdentifierState, doesNotAcceptSpaces) {
	IdentifierState state("identifier", "123");

	ASSERT_THROW(state.nextStateForCharacter(' '), std::runtime_error);
}

TEST(StringLiteralState, consumesSpaces) {
	StringLiteralState state("stringLiteral", "123");

	ASSERT_THAT(state.getTokenId(), Eq("123"));
	ASSERT_THAT(state.getName(), Eq("stringLiteral"));

	ASSERT_THAT(state.nextStateForCharacter(' '), Eq(&state));
}

TEST(StringLiteralState, rejectsNewLines) {
    std::string definitionRecord = "\"stringLiteral 123";
	StringLiteralState state("stringLiteral", "123");

	ASSERT_THAT(state.getTokenId(), Eq("123"));
	ASSERT_THAT(state.getName(), Eq("stringLiteral"));

	ASSERT_THROW(state.nextStateForCharacter('\n'), std::runtime_error);
}
