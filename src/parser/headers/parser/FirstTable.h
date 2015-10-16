#ifndef FIRSTTABLE_H_
#define FIRSTTABLE_H_

#include <iostream>
#include <map>
#include <vector>

#include "GrammarSymbol.h"

namespace parser {

class Grammar;

class FirstTable {
public:
    FirstTable(const Grammar& grammar);
    virtual ~FirstTable();

    const std::vector<GrammarSymbol> operator()(const GrammarSymbol& symbol) const;

private:
    void initializeTable(const std::vector<GrammarSymbol>& symbols, const Grammar& grammar);
    bool addFirstSymbol(const GrammarSymbol& firstFor, const GrammarSymbol& firstSymbol);

    std::map<std::string, std::vector<GrammarSymbol>> firstTable { };

    friend std::ostream& operator<<(std::ostream& ostream, const FirstTable& firstTable);
};

}

#endif /* FIRSTTABLE_H_ */
