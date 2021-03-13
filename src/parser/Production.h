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
    std::vector<int> symbolSequence;
    int id;

public:
    Production(const GrammarSymbol& definingSymbol, std::vector<int> symbolSequence, int id);

    auto begin() const -> decltype(symbolSequence.begin());
    auto end() const -> decltype(symbolSequence.end());
    auto size() const -> decltype(symbolSequence.size());

    int getDefiningSymbol() const;
    std::vector<int> producedSequence() const;
    int getId() const;
};

} // namespace parser

#endif // _PRODUCTION_H_
