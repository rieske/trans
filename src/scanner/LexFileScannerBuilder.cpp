#include "LexFileScannerBuilder.h"
#include "scanner/State.h"

#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <memory>

namespace scanner {

const char NEW_STATE = ':';
const char STATE_TRANSITION = '@';
const char CONFIG_COMMENT = '#';
const char IDENTIFIER = '%';
const char STRING_LITERAL = '"';
const char EOL_COMMENT = '/';

std::unique_ptr<State> parseState(std::string stateDefinitionRecord) {
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
            return std::make_unique<IdentifierState>(stateName, tokenId);
        case STRING_LITERAL:
            return std::make_unique<StringLiteralState>(stateName, tokenId);
        case EOL_COMMENT:
            return std::make_unique<EOLCommentState>(stateName);
        default:
            return std::make_unique<State>(stateDefinition, tokenId);
    }
}

std::pair<std::string, std::string> createNamedTransitionPair(std::string transitionDefinitionRecord) {
    std::istringstream transitionDefinitionStream { transitionDefinitionRecord };
    std::string nextStateName;
    transitionDefinitionStream >> nextStateName;
    std::string transitionCharacters;
    transitionDefinitionStream >> transitionCharacters;
    return std::make_pair(nextStateName, transitionCharacters);
}

FiniteAutomaton* LexFileScannerBuilder::fromConfiguration(std::string configPath) {
    std::ifstream configurationFile { configPath };

    if (!configurationFile.is_open()) {
        throw std::invalid_argument("Unable to open configuration file " + configPath);
    }

    std::map<std::string, std::vector<std::pair<std::string, std::string>>> namedStateTransitions;
    State* newState { nullptr };
    State* startState { nullptr };

    std::string configurationLine;
    while (std::getline(configurationFile, configurationLine)) {
        if (configurationLine.empty()) {
            continue;
        }
        auto entryType = configurationLine.front();
        switch (entryType) {
            case NEW_STATE:
                // belongs to builder.addNewState()
                newState = addNewState(parseState(configurationLine.substr(1)));
                if (startState == nullptr) {
                    startState = newState;
                }
                break;
            case STATE_TRANSITION:
                // builder.addTransition()
                namedStateTransitions[newState->getName()].push_back(createNamedTransitionPair(configurationLine.substr(1)));
                break;
            case IDENTIFIER:
                // builder.addTransition()
                parseKeywords(configurationLine.substr(1));
                break;
            case CONFIG_COMMENT:
                break;
            default:
                throw std::runtime_error("Illegal configuration entry: " + configurationLine);
        }
    }

    // belongs to builder.build()
    for (auto& namedState : namedStates) {
        auto& state = namedState.second;
        auto transitionsAtState = namedStateTransitions.find(state->getName());
        if (transitionsAtState != namedStateTransitions.end()) {
            for (auto& namedTransition : transitionsAtState->second) {
                state->addTransition(namedTransition.second, namedStates.at(namedTransition.first).get());
            }
        }
    }

    return new FiniteAutomaton(startState, keywordIds, std::move(namedStates));
}

State* LexFileScannerBuilder::addNewState(std::unique_ptr<State> stateToAdd) {
    std::string addedStateName = stateToAdd->getName();
    namedStates[addedStateName] = std::move(stateToAdd);
    return namedStates[addedStateName].get();
}

void LexFileScannerBuilder::parseKeywords(std::string keywordsRecord) {
    std::istringstream keywordsStream { keywordsRecord };
    std::string keyword;
    while (keywordsStream >> keyword) {
        // builder.registerKeyword()
        keywordIds[keyword] = nextKeywordId;
        ++nextKeywordId;
    }
}

} // namespace scanner
