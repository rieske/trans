#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <ostream>
#include <vector>

#include "GrammarSymbol.h"
#include "Production.h"

namespace parser {

class Grammar {
public:
    virtual ~Grammar();

    virtual std::size_t ruleCount() const = 0;
    virtual const Production& getRuleByIndex(int index) const = 0;
    virtual std::vector<Production> getProductionsOfSymbol(const GrammarSymbol& symbol) const = 0;

    virtual std::vector<GrammarSymbol> getTerminals() const = 0;
    virtual std::vector<GrammarSymbol> getNonterminals() const = 0;
    const GrammarSymbol& getStartSymbol() const;
    const GrammarSymbol& getEndSymbol() const;

protected:
    GrammarSymbol startSymbol { "<__start__>" };

private:
    const GrammarSymbol endSymbol { "'$end$'" };
};

std::ostream& operator<<(std::ostream& out, const Grammar& grammar);

} // namespace parser

#endif // _GRAMMAR_H_
