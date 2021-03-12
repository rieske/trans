#ifndef FIRSTTABLE_H_
#define FIRSTTABLE_H_

#include <ostream>
#include <map>
#include <vector>

#include "Grammar.h"

namespace parser {

class FirstTable {
public:
    FirstTable(const Grammar& grammar);

    const std::vector<int> operator()(int symbolId) const;
    std::string str(const Grammar& grammar) const;

private:
    void initializeTable(const std::vector<GrammarSymbol>& symbols, const Grammar& grammar);
    bool addFirstSymbol(int firstFor, int firstSymbol);

    std::map<int, std::vector<int>> firstTable { };
};

} // namespace parser

#endif // FIRSTTABLE_H_
