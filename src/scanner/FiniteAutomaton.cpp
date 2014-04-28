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
			accumulator.clear();
			currentState = startState->nextStateForCharacter(inputSymbol);
			if (currentState != startState) {
				accumulator = inputSymbol;
			}
		} else {
			currentState = startState->nextStateForCharacter(inputSymbol);
			accumulator = inputSymbol;
			accumulatedTokenId = -1;
		}
	} else {
		if (nextState == startState) {
			accumulator.clear();
			accumulatedToken.clear();
		} else {
			accumulator += inputSymbol;
		}
		accumulatedTokenId = -1;
		currentState = nextState;
	}
}

bool FiniteAutomaton::isAtFinalState() const {
	return accumulatedTokenId != -1;
}

Token FiniteAutomaton::getCurrentToken() {
	return {accumulatedTokenId, accumulatedToken};
}
