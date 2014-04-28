#include "State.h"

#include <stdexcept>
#include <utility>

using std::shared_ptr;
using std::string;

State::State(string stateName, int tokenId) :
		stateName { stateName },
		tokenId { tokenId },
		wildcardTransition { nullptr } {
}

State::~State() {
}

string State::getName() const {
	return stateName;
}

void State::addTransition(std::string charactersForTransition, std::shared_ptr<State> state) {
	if (charactersForTransition.empty()) {
		wildcardTransition = state;
	} else {
		for (char characterForTransition : charactersForTransition) {
			transitions[characterForTransition] = state;
		}
	}
}

const std::shared_ptr<const State> State::nextStateForCharacter(char c) const {
	if (transitions.find(c) != transitions.end()) {
		return transitions.at(c);
	}
	if (wildcardTransition != nullptr) {
		return wildcardTransition;
	}
	throw std::runtime_error { "Can't reach next state for given input: " + string { c } };
}

void State::outputState(std::ostream& ostream) const {
	ostream << stateName << std::endl;
	for (const std::pair<char, std::shared_ptr<State>>& transition : transitions) {
		ostream << "\t" << transition.first << "\t->\t" << transition.second->getName() << std::endl;
	}
	if (wildcardTransition != nullptr) {
		ostream << wildcardTransition->getName();
	}
}

int State::getTokenId() const {
	return tokenId;
}

bool State::needsKeywordLookup() const {
	return false;
}

bool State::isFinal() const {
	return wildcardTransition == nullptr && transitions.empty();
}
