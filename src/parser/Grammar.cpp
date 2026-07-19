#include "Grammar.h"

#include <algorithm>
#include <sstream>

#include "scanner/Token.h"

namespace parser {

Grammar::Grammar(std::map<std::string, int> symbolIDs,
        std::vector<int> terminals,
        std::vector<int> nonterminals,
        std::vector<Production> rules):
    symbolIDs{std::move(symbolIDs)},
    nonterminalIDs{std::move(nonterminals)},
    terminalIDs{std::move(terminals)},
    startSymbol { -1 },
    endSymbol { 0 },
    topRule{startSymbol, {nonterminalIDs[0]}, static_cast<int>(rules.size())}
{
    int maxId = 0;
    for (const auto& entry : this->symbolIDs) {
        maxId = std::max(maxId, entry.second);
    }
    endSymbol = maxId + 1;

    this->symbolIDs.insert({"<__start__>", startSymbol});
    this->symbolIDs.insert({scanner::Token::END, endSymbol});

    firstTerminalId = this->terminalIDs.empty() ? endSymbol : this->terminalIDs[0];
    this->terminalIDs.push_back(endSymbol);

    rules.push_back(topRule);
    for (const auto& production: rules) {
        rulesByDefiningSymbol.insert({production.getDefiningSymbol(), {}}).first->second.push_back(production);
        rulesById.insert({production.getId(), production});
    }

    idToTerminalBit_.fill(-1);
    terminalBitToId_.reserve(this->terminalIDs.size());
    for (const int terminalId : this->terminalIDs) {
        if (terminalId < 0 || static_cast<std::size_t>(terminalId) >= idToTerminalBit_.size()) {
            throw std::runtime_error { "terminal symbol id out of mapping range" };
        }
        if (idToTerminalBit_[static_cast<std::size_t>(terminalId)] >= 0) {
            continue;
        }
        if (terminalBitToId_.size() >= LOOKAHEAD_BITSET_SIZE) {
            throw std::runtime_error { "too many terminals for lookahead bitset" };
        }
        idToTerminalBit_[static_cast<std::size_t>(terminalId)] = static_cast<int>(terminalBitToId_.size());
        terminalBitToId_.push_back(terminalId);
    }
}

std::size_t Grammar::ruleCount() const {
    return rulesById.size();
}

const Production& Grammar::getTopRule() const {
    return topRule;
}

const Production& Grammar::getRuleById(int index) const {
    return rulesById.at(index);
}

const std::vector<Production>& Grammar::getProductionsOfSymbol(int symbolId) const {
    return rulesByDefiningSymbol.at(symbolId);
}

const std::vector<int>& Grammar::getTerminalIDs() const {
    return terminalIDs;
}

const std::vector<int>& Grammar::getNonterminalIDs() const {
    return nonterminalIDs;
}

int Grammar::getStartSymbol() const {
    return startSymbol;
}

int Grammar::getEndSymbol() const {
    return endSymbol;
}

std::string Grammar::getSymbolById(int symbolId) const {
    auto symbolIt = std::find_if(symbolIDs.begin(), symbolIDs.end(), [&](const std::pair<std::string, int>& pair) {
        return pair.second == symbolId;
    });
    return symbolIt->first;
}

int Grammar::symbolId(std::string definition) const {
    return symbolIDs.at(definition);
}

bool Grammar::isTerminal(int symbolId) const {
    return symbolId >= firstTerminalId;
}

int Grammar::terminalBit(int symbolId) const {
    if (symbolId < 0 || static_cast<std::size_t>(symbolId) >= idToTerminalBit_.size()) {
        throw std::out_of_range { "symbol id out of range for terminal bit map" };
    }
    const int bit = idToTerminalBit_[static_cast<std::size_t>(symbolId)];
    if (bit < 0) {
        throw std::out_of_range { "symbol is not a terminal" };
    }
    return bit;
}

int Grammar::terminalIdFromBit(std::size_t bit) const {
    return terminalBitToId_.at(bit);
}

LookaheadSet Grammar::toLookaheadBits(const std::vector<int>& symbolIds) const {
    LookaheadSet bits;
    for (const int id : symbolIds) {
        bits.set(static_cast<std::size_t>(terminalBit(id)));
    }
    return bits;
}

std::vector<int> Grammar::toTerminalIds(const LookaheadSet& bits) const {
    std::vector<int> ids;
    ids.reserve(bits.count());
    for (std::size_t bit = 0; bit < terminalBitToId_.size(); ++bit) {
        if (bits.test(bit)) {
            ids.push_back(terminalBitToId_[bit]);
        }
    }
    return ids;
}

std::string Grammar::str(int symbolId) const {
    return getSymbolById(symbolId);
}

std::string Grammar::str(const Production& production) const {
    std::stringstream s;
    s << str(production.getDefiningSymbol()) << " ::= ";
    for (const auto& symbol: production) {
        s << str(symbol) << " ";
    }
    auto productionStr = s.str();
    return productionStr.substr(0, productionStr.length()-1);
}

std::ostream& operator<<(std::ostream& out, const Grammar& grammar) {
    out << "\nTerminals:\n";
    for (auto& terminal : grammar.getTerminalIDs()) {
        out << terminal << ":" << grammar.str(terminal) << "\n";
    }
    out << "\nNonterminals:\n";
    for (auto& nonterminal : grammar.getNonterminalIDs()) {
        out << nonterminal << ":" << grammar.str(nonterminal) << "\n";
    }
    out << "\nRules total: " << grammar.ruleCount() << "\n";
    return out;
}

} // namespace parser
