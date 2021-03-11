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
    virtual ~Grammar();

    virtual std::size_t ruleCount() const = 0;
    virtual const Production& getRuleByIndex(int index) const = 0;
    virtual std::vector<Production> getProductionsOfSymbol(const GrammarSymbol& symbol) const = 0;

    virtual std::vector<GrammarSymbol> getTerminals() const = 0;
    virtual std::vector<GrammarSymbol> getNonterminals() const = 0;
    const GrammarSymbol& getStartSymbol() const;
    const GrammarSymbol& getEndSymbol() const;

    std::string getSymbolById(int symbolId) const;

    int symbolId(std::string definition) const;

    std::string str(const GrammarSymbol& symbol) const;

protected:
    int createOrGetSymbolId(std::string definition);

private:
    GrammarSymbol startSymbol;
    GrammarSymbol endSymbol;

    std::map<std::string, int> symbolIDs;
};

std::ostream& operator<<(std::ostream& out, const Grammar& grammar);


} // namespace parser

#endif // _GRAMMAR_H_
