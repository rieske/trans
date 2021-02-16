#ifndef _PRODUCTION_H_
#define _PRODUCTION_H_

#include <initializer_list>
#include <string>
#include <vector>

#include "GrammarSymbol.h"

namespace parser {

class Production {
private:
    GrammarSymbol definingSymbol;
    std::vector<GrammarSymbol> symbolSequence;
    std::size_t id;

public:
    Production(const GrammarSymbol& definingSymbol, std::initializer_list<GrammarSymbol> symbolSequence, std::size_t id);
    Production(const GrammarSymbol& definingSymbol, std::vector<GrammarSymbol> symbolSequence, std::size_t id);

    auto begin() const -> decltype(symbolSequence.begin());
    auto end() const -> decltype(symbolSequence.end());
    auto size() const -> decltype(symbolSequence.size());

    bool produces(const std::vector<std::string>& sequence) const;

    const GrammarSymbol& getDefiningSymbol() const;
    std::vector<std::string> producedSequence() const;
    std::size_t getId() const;
};

} // namespace parser

#endif // _PRODUCTION_H_
