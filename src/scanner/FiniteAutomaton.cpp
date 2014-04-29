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
	accumulatedToken.clear();
	accumulatedTokenId = 0;

	auto nextState = currentState->nextStateForCharacter(inputSymbol);
	if (nextState->isFinal()) {
		accumulatedTokenId = currentState->getTokenId();
		if (currentState->needsKeywordLookup() && (keywordIds.find(accumulator) != keywordIds.end())) {
			accumulatedTokenId = keywordIds.at(accumulator);
		}
		if (accumulatedTokenId != 0) {
			accumulatedToken = accumulator;
		}
		accumulator.clear();
		currentState = startState->nextStateForCharacter(inputSymbol);
	} else {
		currentState = nextState;
	}

	if (currentState != startState) {
		accumulator += inputSymbol;
	}
}

bool FiniteAutomaton::isAtFinalState() const {
	return accumulatedTokenId != 0;
}

Token FiniteAutomaton::getCurrentToken() {
	return {accumulatedTokenId, accumulatedToken};
}
