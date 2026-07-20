#ifndef FIRSTTABLE_H_
#define FIRSTTABLE_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "Grammar.h"

namespace parser {

class FirstTable {
public:
    explicit FirstTable(const Grammar& grammar);

    std::vector<int> operator()(int symbolId) const;
    const LookaheadSet& firstBits(int symbolId) const;
    std::string str(const Grammar& grammar) const;

private:
    void initialize(const Grammar& grammar);
    void addTerminal(int symbol, int terminal);

    const Grammar* grammar_ { nullptr };
    std::unordered_map<int, LookaheadSet> bitsBySymbol;
};

} // namespace parser

#endif
