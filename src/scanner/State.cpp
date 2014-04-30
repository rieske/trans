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

int State::getTokenId() const {
	return tokenId;
}

bool State::needsKeywordLookup() const {
	return false;
}

bool State::isFinal() const {
	return wildcardTransition == nullptr && transitions.empty();
}

std::ostream& operator<<(std::ostream& ostream, const State& state) {
	ostream << state.stateName << ":" << std::endl;
	for (const auto& transition : state.transitions) {
		ostream << "\t" << transition.first << "\t->\t" << transition.second->getName() << std::endl;
	}
	if (state.wildcardTransition != nullptr) {
		ostream << "\t<any character>\t->\t" << state.wildcardTransition->getName() << std::endl;
	}
	return ostream;
}
