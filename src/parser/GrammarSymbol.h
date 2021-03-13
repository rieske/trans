#ifndef _GRAMMARSYMBOL_H_
#define _GRAMMARSYMBOL_H_

#include <vector>

namespace parser {

class GrammarSymbol {
public:
    GrammarSymbol(int symbolId, const std::vector<int>& ruleIndexes);

    int getId() const;
    const std::vector<int>& getRuleIndexes() const;
private:
    int id;
    std::vector<int> ruleIndexes;
};

} // namespace parser

#endif // _GRAMMARSYMBOL_H_
