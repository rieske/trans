#include "FiniteAutomaton.h"

#include "State.h"

using std::string;
using std::map;

FiniteAutomaton::FiniteAutomaton(State* startState, map<string, unsigned> keywordIds) :
		startState { startState },
		currentState { startState },
		keywordIds { keywordIds } {
}

FiniteAutomaton::~FiniteAutomaton() {
}

void FiniteAutomaton::updateState(char inputSymbol) {
	accumulatedLexeme.clear();
	accumulatedToken.clear();

	auto nextState = currentState->nextStateForCharacter(inputSymbol);
	if (nextState->isFinal()) {
		accumulatedToken = currentState->getTokenId();
		if (currentState->needsKeywordLookup() && (keywordIds.find(accumulator) != keywordIds.end())) {
			accumulatedToken = accumulator;
		}
		if (!accumulatedToken.empty()) {
			accumulatedLexeme = accumulator;
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
	return !accumulatedToken.empty();
}

string FiniteAutomaton::getAccumulatedLexeme() const {
	return accumulatedLexeme;
}

string FiniteAutomaton::getAccumulatedToken() const {
	return accumulatedToken;
}
