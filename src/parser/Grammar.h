#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <ostream>
#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include "GrammarSymbol.h"
#include "Production.h"

namespace parser {

class Grammar {
public:
    Grammar(std::map<std::string, int> symbolIDs,
            std::vector<GrammarSymbol> terminals,
            std::vector<GrammarSymbol> nonterminals,
            std::vector<Production> rules);

    std::size_t ruleCount() const;
    const Production& getRuleByIndex(int index) const;
    const std::vector<Production>& getProductionsOfSymbol(int symbolId) const;

    std::vector<int> getTerminalIDs() const;
    std::vector<int> getNonterminalIDs() const;

    const GrammarSymbol& getStartSymbol() const;
    const GrammarSymbol& getEndSymbol() const;

    std::string getSymbolById(int symbolId) const;
    int symbolId(std::string definition) const;

    bool isTerminal(int symbolId) const;

    std::string str(int symbolId) const;
    std::string str(const Production& production) const;

protected:
    std::map<std::string, int> symbolIDs;

    std::vector<GrammarSymbol> terminals;
    std::vector<GrammarSymbol> nonterminals;
    std::vector<Production> rules;

    std::vector<int> nonterminalIDs;
    std::vector<int> terminalIDs;
    std::unordered_map<int, std::vector<Production>> symbolProductions;
private:
    GrammarSymbol startSymbol;
    GrammarSymbol endSymbol;
};

std::ostream& operator<<(std::ostream& out, const Grammar& grammar);

} // namespace parser

#endif // _GRAMMAR_H_
