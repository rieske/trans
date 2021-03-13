#include "Production.h"

#include <stdexcept>

namespace parser {

Production::Production(int definingSymbol, std::vector<int> symbolSequence, int id) :
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

int Production::getDefiningSymbol() const {
    return definingSymbol;
}

std::vector<int> Production::producedSequence() const {
    return symbolSequence;
}

int Production::getId() const {
    return id;
}

} // namespace parser

