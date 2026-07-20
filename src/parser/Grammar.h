#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <array>
#include <bitset>
#include <map>
#include <ostream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "Production.h"

namespace parser {

// Dense terminal bits for LR(1) lookaheads (0 .. terminalCount-1).
constexpr std::size_t LOOKAHEAD_BITSET_SIZE = 128;
using LookaheadSet = std::bitset<LOOKAHEAD_BITSET_SIZE>;

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

    const std::vector<int>& getTerminalIDs() const;
    const std::vector<int>& getNonterminalIDs() const;

    int getStartSymbol() const;
    int getEndSymbol() const;

    std::string getSymbolById(int symbolId) const;
    int symbolId(std::string definition) const;

    bool isTerminal(int symbolId) const;

    // Dense terminal index for lookahead bitsets (0 .. terminalCount()-1).
    std::size_t terminalCount() const noexcept { return terminalBitToId_.size(); }
    int terminalBit(int symbolId) const;
    int terminalIdFromBit(std::size_t bit) const;
    LookaheadSet toLookaheadBits(const std::vector<int>& symbolIds) const;
    std::vector<int> toTerminalIds(const LookaheadSet& bits) const;

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

    std::vector<int> terminalBitToId_;
    std::array<int, 512> idToTerminalBit_ {}; // -1 if not a terminal; covers compact ids
};

std::ostream& operator<<(std::ostream& out, const Grammar& grammar);

} // namespace parser

#endif
