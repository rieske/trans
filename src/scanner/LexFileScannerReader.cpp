#include "LexFileScannerReader.h"
#include "scanner/ScannerBuilder.h"
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

void parseTransition(std::string fromState, std::string transitionDefinitionRecord, ScannerBuilder& builder) {
    std::istringstream transitionDefinitionStream { transitionDefinitionRecord };
    std::string toState;
    transitionDefinitionStream >> toState;
    std::string transitionCharacters;
    transitionDefinitionStream >> transitionCharacters;
    builder.addTransition(fromState, transitionCharacters, toState);
}

void parseKeywords(std::string keywordsRecord, ScannerBuilder& builder) {
    std::istringstream keywordsStream { keywordsRecord };
    std::string keyword;
    while (keywordsStream >> keyword) {
        builder.addKeyword(keyword);
    }
}

void parseConfigurationEntry(char entryType, std::string entry, std::string& currentState, ScannerBuilder& builder) {
    switch (entryType) {
        case NEW_STATE:
            currentState = builder.addState(parseState(entry));
            break;
        case STATE_TRANSITION:
            parseTransition(currentState, entry, builder);
            break;
        case IDENTIFIER:
            parseKeywords(entry, builder);
            break;
        case CONFIG_COMMENT:
            break;
        default:
            throw std::runtime_error("Illegal scanner configuration entry: " + entry);
    }
}

FiniteAutomaton* LexFileScannerReader::fromConfiguration(std::string configPath) {
    std::ifstream configurationFile { configPath };

    if (!configurationFile.is_open()) {
        throw std::invalid_argument("Unable to open configuration file " + configPath);
    }

    ScannerBuilder builder;
    std::string configurationLine;
    std::string currentState;
    while (std::getline(configurationFile, configurationLine)) {
        if (configurationLine.empty()) {
            continue;
        }
        auto entryType = configurationLine.front();
        parseConfigurationEntry(entryType, configurationLine.substr(1), currentState, builder);
    }

    return builder.build();
}

} // namespace scanner

