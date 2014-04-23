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
	if (nextState == nullptr) {
		accumulator.clear();
		nextState = startState->nextStateForCharacter(inputSymbol);
	}

	if (nextState == nullptr) {
		currentState = startState;
	} else if (nextState == finalState) {
		accumulatedToken = accumulator;
		accumulatedTokenId = currentState->getTokenId();
		if (currentState->needsKeywordLookup() && (keywordIds.find(accumulatedToken) != keywordIds.end())) {
			accumulatedTokenId = keywordIds.at(accumulatedToken);
		}
		accumulator = inputSymbol;
		currentState = finalState;
	} else {
		accumulator += inputSymbol;
		currentState = nextState;
	}
}

bool FiniteAutomaton::isAtFinalState() const {
	return currentState == finalState;
}

Token FiniteAutomaton::getCurrentToken() {
	return {accumulatedTokenId, accumulatedToken};
}
