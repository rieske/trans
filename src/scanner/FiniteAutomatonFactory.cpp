#include "FiniteAutomatonFactory.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "FiniteAutomaton.h"
#include "State.h"

using std::string;
using std::unique_ptr;
using std::shared_ptr;
using std::ifstream;
using std::map;
using std::vector;
using std::pair;

const char NEW_STATE = ':';
const char STATE_TRANSITION = '@';
const char KEYWORD = '%';
const char CONFIG_COMMENT = '#';

FiniteAutomatonFactory::FiniteAutomatonFactory(string configurationFileName) :
		configurationFile { configurationFileName } {
	if (!configurationFile.is_open()) {
		throw std::invalid_argument("Unable to open configuration file " + configurationFileName);
	}

	map<string, vector<pair<string, string>>> namedStateTransitions;
	shared_ptr<State> currentState { nullptr };

	string configurationLine;
	while (std::getline(configurationFile, configurationLine)) {
		if (configurationLine.empty()) {
			continue;
		}
		char entryType = configurationLine.at(0);
		switch (entryType) {
		case NEW_STATE:
			currentState = createNewState(configurationLine.substr(1));
			namedStates[currentState->getName()] = currentState;
			break;
		case STATE_TRANSITION:
			namedStateTransitions[currentState->getName()].push_back(createNamedTransitionPair(configurationLine.substr(1)));
			break;
		case KEYWORD:
			parseKeywords(configurationLine.substr(1));
			break;
		case CONFIG_COMMENT:
			break;
		default:
			throw std::runtime_error("Illegal configuration entry: " + configurationLine);
		}
	}
	finalState = currentState;
	for (pair<string, shared_ptr<State>> namedState : namedStates) {
		shared_ptr<State> state = namedState.second;
		if (state != finalState) {
			vector<pair<string, string>> namedTransitions = namedStateTransitions.at(state->getName());
			for (pair<string, string> namedTransition : namedTransitions) {
				state->addTransition(namedTransition.second, namedStates.at(namedTransition.first));
			}
		}
	}
}

FiniteAutomatonFactory::~FiniteAutomatonFactory() {
	configurationFile.close();
}

unique_ptr<StateMachine> FiniteAutomatonFactory::createAutomaton() const {
	return unique_ptr<StateMachine> { new FiniteAutomaton { startState, finalState, keywordIds } };
}

shared_ptr<State> FiniteAutomatonFactory::createNewState(string stateDefinitionRecord) {
	shared_ptr<State> state { new State { stateDefinitionRecord, nextStateId } };
	++nextStateId;
	if (startState == nullptr) {
		startState = state;
	}
	return state;
}

pair<string, string> FiniteAutomatonFactory::createNamedTransitionPair(string transitionDefinitionRecord) {
	std::istringstream transitionDefinitionStream { transitionDefinitionRecord };
	string nextStateName;
	transitionDefinitionStream >> nextStateName;
	string transitionCharacters;
	transitionDefinitionStream >> transitionCharacters;
	return std::make_pair(nextStateName, transitionCharacters);
}

void FiniteAutomatonFactory::parseKeywords(std::string keywordsRecord) {
	std::istringstream keywordsStream { keywordsRecord };
	string keyword;
	while (keywordsStream >> keyword) {
		keywordIds[keyword] = nextKeywordId;
		++nextKeywordId;
	}
}
