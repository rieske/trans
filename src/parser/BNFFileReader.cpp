#include "BNFFileReader.h"
#include "parser/GrammarBuilder.h"

#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <map>

#include <iostream>

namespace {

const char NONTERMINAL_START = '<';
const char NONTERMINAL_END = '>';
const char TERMINAL_START = '\'';
const char TERMINAL_END = '\'';

} // namespace

namespace parser {

Grammar BNFFileReader::readGrammar(const std::string bnfFileName) const {
    std::ifstream bnfInputStream { bnfFileName };
    if (!bnfInputStream.is_open()) {
        throw std::invalid_argument("Unable to open bnf file for reading: " + bnfFileName);
    }

    std::vector<std::string> symbolsBeingDefined;
    std::string nonterminalName;
    std::vector<std::string> producedSymbolNames;

    GrammarBuilder builder;

    for (std::string bnfToken; bnfInputStream >> bnfToken;) {
        if (bnfToken.length() == 1) {
            switch (bnfToken.front()) {
                case '|': {
                    builder.defineRule(nonterminalName, producedSymbolNames);
                    producedSymbolNames.clear();
                    break;
                }
                case ';': {
                    builder.defineRule(nonterminalName, producedSymbolNames);
                    producedSymbolNames.clear();
                    nonterminalName.clear();
                    break;
                }
                case ':':
                    break;
                default:
                    throw std::runtime_error("Unrecognized control character in grammar configuration file: " + bnfToken);
            }
        } else if (!bnfToken.empty() && bnfToken.front() == NONTERMINAL_START && bnfToken.back() == NONTERMINAL_END) {
            const auto& nonterminalBeingDefinedIterator = std::find(symbolsBeingDefined.begin(), symbolsBeingDefined.end(), bnfToken);
            if (nonterminalBeingDefinedIterator == symbolsBeingDefined.end()) {
                symbolsBeingDefined.push_back(bnfToken);
            }
            if (!nonterminalName.empty()) {
                producedSymbolNames.push_back(bnfToken);
            } else {
                nonterminalName = bnfToken;
            }
        } else if (!bnfToken.empty() && bnfToken.front() == TERMINAL_START && bnfToken.back() == TERMINAL_END) {
            producedSymbolNames.push_back(bnfToken.substr(1, bnfToken.size() - 2));
        } else {
            throw std::runtime_error("Unrecognized token in grammar configuration file: " + bnfToken);
        }
    }

    return builder.build();
}

} // namespace parser

