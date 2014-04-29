#include "FiniteAutomatonFactory.h"

#include <sstream>
#include <fstream>
#include <stdexcept>
#include <vector>

#include "EOLCommentState.h"
#include "FiniteAutomaton.h"
#include "IdentifierState.h"
#include "StringLiteralState.h"

using std::string;
using std::unique_ptr;
using std::shared_ptr;
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

FiniteAutomatonFactory::FiniteAutomatonFactory(string configurationFileName) {
	ifstream configurationFile { configurationFileName };

	if (!configurationFile.is_open()) {
		throw std::invalid_argument("Unable to open configuration file " + configurationFileName);
	}

	map<string, vector<pair<string, string>>> namedStateTransitions;
	shared_ptr<State> currentState { nullptr };
	std::map<std::string, std::shared_ptr<State>> namedStates;

	string configurationLine;
	while (std::getline(configurationFile, configurationLine)) {
		if (configurationLine.empty()) {
			continue;
		}
		auto entryType = configurationLine.at(0);
		switch (entryType) {
		case NEW_STATE:
			currentState = createNewState(configurationLine.substr(1));
			namedStates[currentState->getName()] = currentState;
			if (startState == nullptr) {
				startState = currentState;
			}
			break;
		case STATE_TRANSITION:
			namedStateTransitions[currentState->getName()].push_back(createNamedTransitionPair(configurationLine.substr(1)));
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
	configurationFile.close();

	for (auto namedState : namedStates) {
		auto state = namedState.second;
		if (namedStateTransitions.find(state->getName()) != namedStateTransitions.end()) {
			auto namedTransitions = namedStateTransitions.at(state->getName());
			for (auto namedTransition : namedTransitions) {
				state->addTransition(namedTransition.second, namedStates.at(namedTransition.first));
			}
		}
	}
}

FiniteAutomatonFactory::~FiniteAutomatonFactory() {
}

unique_ptr<StateMachine> FiniteAutomatonFactory::createAutomaton() const {
	return unique_ptr<StateMachine> { new FiniteAutomaton { startState, keywordIds } };
}

shared_ptr<State> FiniteAutomatonFactory::createNewState(string stateDefinitionRecord) {
	if (stateDefinitionRecord.empty()) {
		throw std::invalid_argument("Empty state record");
	}
	std::istringstream stateDefinitionStream { stateDefinitionRecord };
	string stateDefinition;
	stateDefinitionStream >> stateDefinition;
	int tokenId { 0 };
	stateDefinitionStream >> tokenId;
	string stateName = stateDefinition.substr(1, stateDefinition.length());
	char stateType = stateDefinition.at(0);
	switch (stateType) {
	case IDENTIFIER:
		return shared_ptr<State> { new IdentifierState { stateName, tokenId } };
	case STRING_LITERAL:
		return shared_ptr<State> { new StringLiteralState { stateName, tokenId } };
	case EOL_COMMENT:
		return shared_ptr<State> { new EOLCommentState { stateName } };
	default:
		return shared_ptr<State> { new State { stateDefinition, tokenId } };
	}
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

/*void FiniteAutomatonFactory::logAutomatonConfiguration() const {
	map<string, State*>::const_iterator it;
	for (it = m_state.begin(); it != m_state.end(); ++it) {
		fprintf(logfile, "%s:%d\n", it->first.c_str(), it->second->getTokenId());
		it->second->listing(logfile);
	}

	if (!m_keywords.empty()) {
		fprintf(logfile, "\nKeywords:\n");
		map<string, unsigned int>::const_iterator kw_it;
		for (kw_it = m_keywords.begin(); kw_it != m_keywords.end(); ++kw_it)
			fprintf(logfile, "%s:%d\n", kw_it->first.c_str(), kw_it->second);
	}
}*/
