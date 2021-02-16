#ifndef BNFREADER_H_
#define BNFREADER_H_

#include <memory>
#include <string>
#include <vector>

#include "Grammar.h"

namespace parser {

class BNFFileGrammar: public Grammar {
public:
    BNFFileGrammar(const std::string bnfFileName);
    virtual ~BNFFileGrammar();

    std::size_t ruleCount() const override;
    const Production& getRuleByIndex(int index) const override;
    std::vector<Production> getProductionsOfSymbol(const GrammarSymbol& symbol) const override;

    std::vector<GrammarSymbol> getTerminals() const override;
    std::vector<GrammarSymbol> getNonterminals() const override;

private:
    GrammarSymbol& addSymbol(const std::string& name);

    std::vector<GrammarSymbol> symbols;

    std::vector<Production> rules;

    std::vector<GrammarSymbol> terminals;
    std::vector<GrammarSymbol> nonterminals;
};

} // namespace parser

#endif // BNFREADER_H_
