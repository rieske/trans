#ifndef GRAMMARSYMBOL_H_
#define GRAMMARSYMBOL_H_

#include <iostream>
#include <string>
#include <vector>

#include "Production.h"

namespace parser {

class GrammarSymbol {
public:
    GrammarSymbol(const std::string& definition);
    virtual ~GrammarSymbol();

    void addRuleIndex(std::size_t ruleIndex);
    const std::vector<std::size_t>& getRuleIndexes() const;

    bool isTerminal() const;
    bool isNonterminal() const;

    std::string getDefinition() const;

private:
    std::string definition;
    std::vector<std::size_t> ruleIndexes {};
};

bool operator==(const GrammarSymbol& lhs, const GrammarSymbol& rhs);
bool operator<(const GrammarSymbol& lhs, const GrammarSymbol& rhs);

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol);

}

#endif /* GRAMMARSYMBOL_H_ */
