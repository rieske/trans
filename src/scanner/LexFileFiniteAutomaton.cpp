#include "LexFileFiniteAutomaton.h"

#include <sstream>
#include <fstream>
#include <vector>

#include "EOLCommentState.h"
#include "FiniteAutomaton.h"
#include "IdentifierState.h"
#include "StringLiteralState.h"

namespace scanner {

const char NEW_STATE = ':';
const char STATE_TRANSITION = '@';
const char CONFIG_COMMENT = '#';
const char IDENTIFIER = '%';
const char STRING_LITERAL = '"';
const char EOL_COMMENT = '/';

LexFileFiniteAutomaton::LexFileFiniteAutomaton(std::string configurationFileName) {
    std::ifstream configurationFile { configurationFileName };

    if (!configurationFile.is_open()) {
        throw std::invalid_argument("Unable to open configuration file " + configurationFileName);
    }

    std::map<std::string, std::vector<std::pair<std::string, std::string>>> namedStateTransitions;
    State* newState { nullptr };

    std::string configurationLine;
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
        auto transitionsAtState = namedStateTransitions.find(state->getName());
        if (transitionsAtState != namedStateTransitions.end()) {
            for (auto& namedTransition : transitionsAtState->second) {
                state->addTransition(namedTransition.second, namedStates.at(namedTransition.first).get());
            }
        }
    }

    currentState = startState;
}

LexFileFiniteAutomaton::~LexFileFiniteAutomaton() {
}

State* LexFileFiniteAutomaton::addNewState(std::string stateDefinitionRecord) {
    if (stateDefinitionRecord.empty()) {
        throw std::invalid_argument("Empty state record");
    }
    std::istringstream stateDefinitionStream { stateDefinitionRecord };
    std::string stateDefinition;
    stateDefinitionStream >> stateDefinition;
    std::string tokenId;
    stateDefinitionStream >> tokenId;
    std::string stateName = stateDefinition.substr(1, stateDefinition.length());
    std::unique_ptr<State> stateToAdd;
    char stateType = stateDefinition.front();
    switch (stateType) {
    case IDENTIFIER:
        stateToAdd = std::make_unique<IdentifierState>(stateName, tokenId);
        break;
    case STRING_LITERAL:
        stateToAdd = std::make_unique<StringLiteralState>(stateName, tokenId);
        break;
    case EOL_COMMENT:
        stateToAdd = std::make_unique<EOLCommentState>(stateName);
        break;
    default:
        stateToAdd = std::make_unique<State>(stateDefinition, tokenId);
        break;
    }
    std::string addedStateName = stateToAdd->getName();
    namedStates[addedStateName] = std::move(stateToAdd);
    return namedStates[addedStateName].get();
}

std::pair<std::string, std::string> LexFileFiniteAutomaton::createNamedTransitionPair(std::string transitionDefinitionRecord) {
    std::istringstream transitionDefinitionStream { transitionDefinitionRecord };
    std::string nextStateName;
    transitionDefinitionStream >> nextStateName;
    std::string transitionCharacters;
    transitionDefinitionStream >> transitionCharacters;
    return std::make_pair(nextStateName, transitionCharacters);
}

void LexFileFiniteAutomaton::parseKeywords(std::string keywordsRecord) {
    std::istringstream keywordsStream { keywordsRecord };
    std::string keyword;
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

} // namespace scanner

