#ifndef _PRODUCTION_H_
#define _PRODUCTION_H_

#include <vector>

namespace parser {

class Production {
private:
    int definingSymbol;
    std::vector<int> symbolSequence;
    int id;

public:
    Production(int definingSymbol, std::vector<int> symbolSequence, int id);

    auto begin() const -> decltype(symbolSequence.begin());
    auto end() const -> decltype(symbolSequence.end());
    auto size() const -> decltype(symbolSequence.size());

    int getDefiningSymbol() const;
    const std::vector<int>& producedSequence() const;
    int getId() const;
};

} // namespace parser

#endif // _PRODUCTION_H_
