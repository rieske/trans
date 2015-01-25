#include "Production.h"

#include <cstddef>
#include <stdexcept>

#include "GrammarSymbol.h"

namespace parser {

Production::Production(GrammarSymbol result, std::initializer_list<GrammarSymbol> symbolSequence, std::size_t id) :
        Production(result, std::vector<GrammarSymbol> { symbolSequence }, id)
{
}

Production::Production(GrammarSymbol result, std::vector<GrammarSymbol> symbolSequence, std::size_t id) :
        //result {result},
        symbolSequence { symbolSequence },
        id { id }
{
    if (this->symbolSequence.empty()) {
        throw std::invalid_argument { "can not produce empty symbol sequence" };
    }
}

const auto Production::begin() const -> decltype(symbolSequence.begin()) {
    return symbolSequence.begin();
}

const auto Production::end() const -> decltype(symbolSequence.end()) {
    return symbolSequence.end();
}

auto Production::size() const -> decltype(symbolSequence.size()) {
    return symbolSequence.size();
}

bool Production::produces(const std::vector<std::string>& sequence) const {
    if (sequence.size() != symbolSequence.size()) {
        return false;
    }
    for (size_t i = 0; i != sequence.size(); ++i) {
        if (sequence.at(i) != symbolSequence.at(i).getDefinition()) {
            return false;
        }
    }
    return true;
}

std::vector<std::string> Production::producedSequence() const {
    std::vector<std::string> producedSequence;
    for (const auto& symbol : symbolSequence) {
        producedSequence.push_back(symbol.getDefinition());
    }
    return producedSequence;
}

std::size_t Production::getId() const {
    return id;
}

} /* namespace parser */
