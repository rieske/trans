#include "FiniteAutomaton.h"

#include <iostream>

#include "State.h"
#include "Token.h"

using std::string;
using std::shared_ptr;
using std::map;

FiniteAutomaton::FiniteAutomaton(shared_ptr<State> startState, shared_ptr<State> finalState, map<string, unsigned> keywordIds) :
		tokenState { startState },
		startState { startState },
		currentState { startState },
		finalState { finalState },
		keywordIds(keywordIds) {
}

FiniteAutomaton::~FiniteAutomaton() {
}

void FiniteAutomaton::updateState(char inputSymbol) {
	tokenState = currentState;
	currentState = currentState->nextStateForCharacter(inputSymbol);
	if (currentState == nullptr) {
		currentState = startState;
		accumulator = "";
	} else if (currentState == finalState) {
		accumulatedTokenId = tokenState->getTokenId();
		if (tokenState->isPossibleKeyword() && (keywordIds.find(accumulator) != keywordIds.end())) {
			accumulatedTokenId = keywordIds.at(accumulator);
		}
		accumulatedToken = accumulator;
		accumulator = inputSymbol;
	} else {
		accumulator += inputSymbol;
	}
}

bool FiniteAutomaton::isAtFinalState() const {
	return currentState == finalState;
}

Token FiniteAutomaton::getCurrentToken() {
	Token token(accumulatedTokenId, accumulatedToken);
	accumulatedTokenId = -1;
	accumulatedToken = "";
	currentState = startState;
	return token;
}
