#include "FiniteAutomaton.h"

#include <iostream>

#include "State.h"
#include "Token.h"

using std::string;
using std::shared_ptr;
using std::map;

FiniteAutomaton::FiniteAutomaton(shared_ptr<State> startState, map<string, unsigned> keywordIds) :
		startState { startState },
		currentState { startState },
		keywordIds { keywordIds } {
}

FiniteAutomaton::~FiniteAutomaton() {
}

void FiniteAutomaton::updateState(char inputSymbol) {

	auto nextState = currentState->nextStateForCharacter(inputSymbol);
	if (nextState->isFinal()) {
		if (currentState->needsKeywordLookup() && (keywordIds.find(accumulator) != keywordIds.end())) {
			accumulatedTokenId = keywordIds.at(accumulator);
		} else {
			accumulatedTokenId = currentState->getTokenId();
		}
		if (accumulatedTokenId != 0) {
			accumulatedToken = accumulator;
			currentState = nextState;
		} else {
			currentState = startState->nextStateForCharacter(inputSymbol);
			accumulator = inputSymbol;
		}
	} else {
		if (nextState == startState) {
			accumulator.clear();
		} else {
			accumulator += inputSymbol;
		}
		currentState = nextState;
	}
}

bool FiniteAutomaton::isAtFinalState() const {
	return currentState->isFinal();
}

Token FiniteAutomaton::getCurrentToken() {
	return {accumulatedTokenId, accumulatedToken};
}
