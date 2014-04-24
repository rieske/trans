#include "FiniteAutomaton.h"

#include <iostream>

#include "State.h"
#include "Token.h"

using std::string;
using std::shared_ptr;
using std::map;

FiniteAutomaton::FiniteAutomaton(shared_ptr<State> startState, shared_ptr<State> finalState, map<string, unsigned> keywordIds) :
		startState { startState },
		currentState { startState },
		finalState { finalState },
		keywordIds { keywordIds } {
}

FiniteAutomaton::~FiniteAutomaton() {
}

void FiniteAutomaton::updateState(char inputSymbol) {

	auto nextState = currentState->nextStateForCharacter(inputSymbol);
	if (nextState == finalState) {
		accumulatedTokenId = currentState->getTokenId();
		if (currentState->needsKeywordLookup() && (keywordIds.find(accumulator) != keywordIds.end())) {
			accumulatedTokenId = keywordIds.at(accumulator);
		}
		if (accumulatedTokenId != 0) {
			accumulatedToken = accumulator;
			currentState = finalState;
		} else {
			currentState = startState->nextStateForCharacter(inputSymbol);
			accumulator = inputSymbol;
		}
	} else {
		accumulator += inputSymbol;
		if (nextState == startState) {
			accumulator.clear();
		}
		currentState = nextState;
	}
}

bool FiniteAutomaton::isAtFinalState() const {
	return currentState == finalState;
}

Token FiniteAutomaton::getCurrentToken() {
	return {accumulatedTokenId, accumulatedToken};
}
