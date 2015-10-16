#include "scanner/LexFileFiniteAutomaton.h"

#include <sstream>
#include <fstream>
#include <stdexcept>
#include <vector>

#include "scanner/EOLCommentState.h"
#include "scanner/FiniteAutomaton.h"
#include "scanner/IdentifierState.h"
#include "scanner/StringLiteralState.h"

using std::string;
using std::unique_ptr;
using std::ifstream;
using std::map;
using std::vector;
using std::pair;

const char NEW_STATE = ':';
const char STATE_TRANSITION = '@';
const char CONFIG_COMMENT = '#';
const char IDENTIFIER = '%';
const char STRING_LITERAL = '"';
const char EOL_COMMENT = '/';

LexFileFiniteAutomaton::LexFileFiniteAutomaton(string configurationFileName) {
	ifstream configurationFile { configurationFileName };

	if (!configurationFile.is_open()) {
		throw std::invalid_argument("Unable to open configuration file " + configurationFileName);
	}

	map<string, vector<pair<string, string>>> namedStateTransitions;
	State* newState { nullptr };

	string configurationLine;
	while (std::getline(configurationFile, configurationLine)) {
		if (configurationLine.empty()) {
			continue;
		}
		auto entryType = configurationLine.front();
		switch (entryType) {
		case NEW_STATE:
			newState = addNewState(configurationLine.substr(1));
			if (startState == nullptr) {
				startState = newState;
			}
			break;
		case STATE_TRANSITION:
			namedStateTransitions[newState->getName()].push_back(createNamedTransitionPair(configurationLine.substr(1)));
			break;
		case IDENTIFIER:
			parseKeywords(configurationLine.substr(1));
			break;
		case CONFIG_COMMENT:
			break;
		default:
			throw std::runtime_error("Illegal configuration entry: " + configurationLine);
		}
	}

	for (auto& namedState : namedStates) {
		auto& state = namedState.second;
		if (namedStateTransitions.find(state->getName()) != namedStateTransitions.end()) {
			auto namedTransitions = namedStateTransitions.at(state->getName());
			for (auto& namedTransition : namedTransitions) {
				state->addTransition(namedTransition.second, namedStates.at(namedTransition.first).get());
			}
		}
	}

	currentState = startState;
}

LexFileFiniteAutomaton::~LexFileFiniteAutomaton() {
}

State* LexFileFiniteAutomaton::addNewState(string stateDefinitionRecord) {
	if (stateDefinitionRecord.empty()) {
		throw std::invalid_argument("Empty state record");
	}
	std::istringstream stateDefinitionStream { stateDefinitionRecord };
	string stateDefinition;
	stateDefinitionStream >> stateDefinition;
	string tokenId;
	stateDefinitionStream >> tokenId;
	string stateName = stateDefinition.substr(1, stateDefinition.length());
	unique_ptr<State> stateToAdd;
	char stateType = stateDefinition.front();
	switch (stateType) {
	case IDENTIFIER:
		stateToAdd = unique_ptr<State> { new IdentifierState { stateName, tokenId } };
		break;
	case STRING_LITERAL:
		stateToAdd = unique_ptr<State> { new StringLiteralState { stateName, tokenId } };
		break;
	case EOL_COMMENT:
		stateToAdd = unique_ptr<State> { new EOLCommentState { stateName } };
		break;
	default:
		stateToAdd = unique_ptr<State> { new State { stateDefinition, tokenId } };
		break;
	}
	string addedStateName = stateToAdd->getName();
	namedStates[addedStateName] = std::move(stateToAdd);
	return namedStates[addedStateName].get();
}

pair<string, string> LexFileFiniteAutomaton::createNamedTransitionPair(string transitionDefinitionRecord) {
	std::istringstream transitionDefinitionStream { transitionDefinitionRecord };
	string nextStateName;
	transitionDefinitionStream >> nextStateName;
	string transitionCharacters;
	transitionDefinitionStream >> transitionCharacters;
	return std::make_pair(nextStateName, transitionCharacters);
}

void LexFileFiniteAutomaton::parseKeywords(std::string keywordsRecord) {
	std::istringstream keywordsStream { keywordsRecord };
	string keyword;
	while (keywordsStream >> keyword) {
		keywordIds[keyword] = nextKeywordId;
		++nextKeywordId;
	}
}

std::ostream& operator<<(std::ostream& os, const LexFileFiniteAutomaton& factory) {
	for (const auto& namedState : factory.namedStates) {
		os << *namedState.second << std::endl;
	}
	os << "Keywords:\n";
	for (const auto& keywordToId : factory.keywordIds) {
		os << "\t" << keywordToId.first << " " << keywordToId.second << std::endl;
	}
	return os;
}
