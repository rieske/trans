#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <ostream>
#include <vector>
#include <map>
#include <unordered_map>

#include "Production.h"

namespace parser {

class Grammar {
public:
    Grammar(std::map<std::string, int> symbolIDs,
            std::vector<int> terminals,
            std::vector<int> nonterminals,
            std::vector<Production> rules);

    std::size_t ruleCount() const;
    const Production& getTopRule() const;
    const Production& getRuleById(int index) const;
    const std::vector<Production>& getProductionsOfSymbol(int symbolId) const;

    std::vector<int> getTerminalIDs() const;
    std::vector<int> getNonterminalIDs() const;

    int getStartSymbol() const;
    int getEndSymbol() const;

    std::string getSymbolById(int symbolId) const;
    int symbolId(std::string definition) const;

    bool isTerminal(int symbolId) const;

    std::string str(int symbolId) const;
    std::string str(const Production& production) const;

private:
    std::map<std::string, int> symbolIDs;

    std::vector<int> nonterminalIDs;
    std::vector<int> terminalIDs;
    std::unordered_map<int, std::vector<Production>> rulesByDefiningSymbol;
    std::unordered_map<int, Production> rulesById;

    int firstTerminalId;
    int startSymbol;
    int endSymbol;
    Production topRule;
};

std::ostream& operator<<(std::ostream& out, const Grammar& grammar);

} // namespace parser

#endif // _GRAMMAR_H_
