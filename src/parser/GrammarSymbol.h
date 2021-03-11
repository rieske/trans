#ifndef _GRAMMARSYMBOL_H_
#define _GRAMMARSYMBOL_H_

#include <ostream>
#include <vector>

namespace parser {

class GrammarSymbol {
public:
    GrammarSymbol(int symbolId, const std::vector<int>& ruleIndexes = { });

    int getId() const;
    const std::vector<int>& getRuleIndexes() const;

    bool isTerminal() const;
    bool isNonterminal() const;

private:
    int id;
    std::vector<int> ruleIndexes;
};

bool operator==(const GrammarSymbol& lhs, const GrammarSymbol& rhs);
bool operator<(const GrammarSymbol& lhs, const GrammarSymbol& rhs);

} // namespace parser

#endif // _GRAMMARSYMBOL_H_
