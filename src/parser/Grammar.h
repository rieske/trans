#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <iostream>
#include <memory>
#include <vector>

#include "GrammarSymbol.h"

namespace parser {

class Grammar {
public:
    virtual ~Grammar();

    virtual std::size_t ruleCount() const = 0;
    virtual const Production& getRuleByIndex(int index) const = 0;
    virtual std::vector<Production> getProductionsOfSymbol(const GrammarSymbol* symbol) const = 0;

    virtual std::vector<const GrammarSymbol*> getTerminals() const = 0;
    virtual std::vector<const GrammarSymbol*> getNonterminals() const = 0;
    const GrammarSymbol* getStartSymbol() const;
    const GrammarSymbol* getEndSymbol() const;

protected:
    const std::unique_ptr<GrammarSymbol> startSymbol { new GrammarSymbol("<__start__>") };

private:
    const std::unique_ptr<GrammarSymbol> endSymbol { new GrammarSymbol("'$end$'") };
};

std::ostream& operator<<(std::ostream& out, const Grammar& grammar);

}

#endif // _GRAMMAR_H_
