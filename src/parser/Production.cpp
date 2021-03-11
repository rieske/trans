#include "Production.h"

#include <stdexcept>

namespace parser {

Production::Production(const GrammarSymbol& definingSymbol, std::initializer_list<GrammarSymbol> symbolSequence, std::size_t id) :
        Production(definingSymbol, std::vector<GrammarSymbol> { symbolSequence }, id)
{
}

Production::Production(const GrammarSymbol& definingSymbol, std::vector<GrammarSymbol> symbolSequence, std::size_t id) :
        definingSymbol {definingSymbol},
        symbolSequence { symbolSequence },
        id { id }
{
    if (this->symbolSequence.empty()) {
        throw std::invalid_argument { "can not produce empty symbol sequence" };
    }
}

auto Production::begin() const -> decltype(symbolSequence.begin()) {
    return symbolSequence.begin();
}

auto Production::end() const -> decltype(symbolSequence.end()) {
    return symbolSequence.end();
}

auto Production::size() const -> decltype(symbolSequence.size()) {
    return symbolSequence.size();
}

const GrammarSymbol& Production::getDefiningSymbol() const {
    return definingSymbol;
}

std::vector<int> Production::producedSequence() const {
    std::vector<int> producedSequence;
    for (const auto& symbol : symbolSequence) {
        producedSequence.push_back(symbol.getId());
    }
    return producedSequence;
}

std::size_t Production::getId() const {
    return id;
}

} // namespace parser

