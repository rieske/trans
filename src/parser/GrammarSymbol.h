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

    void addRuleIndex(int ruleIndex);
    const std::vector<int>& getRuleIndexes() const;

    bool isTerminal() const;
    bool isNonterminal() const;

    std::string getDefinition() const;

private:
    const std::string definition;
    std::vector<int> ruleIndexes;
};

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol);

}

#endif /* GRAMMARSYMBOL_H_ */
