#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <ostream>
#include <vector>
#include <map>

#include "GrammarSymbol.h"
#include "Production.h"

namespace parser {

class Grammar {
public:
    Grammar();
    Grammar(std::map<std::string, int> symbolIDs,
            std::vector<GrammarSymbol> terminals,
            std::vector<GrammarSymbol> nonterminals,
            std::vector<Production> rules);
    virtual ~Grammar();

    std::size_t ruleCount() const;
    const Production& getRuleByIndex(int index) const;
    std::vector<Production> getProductionsOfSymbol(const GrammarSymbol& symbol) const;
    std::vector<Production> getProductionsOfSymbol(std::string symbol) const;

    std::vector<GrammarSymbol> getTerminals() const;
    std::vector<GrammarSymbol> getNonterminals() const;
    const GrammarSymbol& getStartSymbol() const;
    const GrammarSymbol& getEndSymbol() const;

    std::string getSymbolById(int symbolId) const;

    int symbolId(std::string definition) const;

    std::string str(const GrammarSymbol& symbol) const;
    std::string str(const Production& production) const;

protected:
    std::map<std::string, int> symbolIDs;

    std::vector<GrammarSymbol> terminals;
    std::vector<GrammarSymbol> nonterminals;
    std::vector<Production> rules;
private:
    GrammarSymbol startSymbol;
    GrammarSymbol endSymbol;
};

std::ostream& operator<<(std::ostream& out, const Grammar& grammar);

} // namespace parser

#endif // _GRAMMAR_H_
