#ifndef _GRAMMARSYMBOL_H_
#define _GRAMMARSYMBOL_H_

#include <ostream>
#include <string>
#include <vector>

namespace parser {

class GrammarSymbol {
public:
    GrammarSymbol(const std::string& definition, const std::vector<std::size_t>& ruleIndexes = { });

    std::string getDefinition() const;
    const std::vector<std::size_t>& getRuleIndexes() const;

    bool isTerminal() const;
    bool isNonterminal() const;

private:
    std::string definition;
    std::vector<std::size_t> ruleIndexes;
};

bool operator==(const GrammarSymbol& lhs, const GrammarSymbol& rhs);
bool operator<(const GrammarSymbol& lhs, const GrammarSymbol& rhs);

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol);

} // namespace parser

#endif // _GRAMMARSYMBOL_H_
