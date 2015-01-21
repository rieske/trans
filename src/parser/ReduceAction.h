#ifndef REDUCEACTION_H_
#define REDUCEACTION_H_

#include <memory>
#include <stack>
#include <string>

#include "Action.h"
#include "LookaheadActionTable.h"
#include "TokenStream.h"

namespace parser {

class GrammarSymbol;

class ReduceAction: public Action {
public:
    ReduceAction(const GrammarSymbol* grammarSymbol, int productionNumber, const ParsingTable* parsingTable);
    virtual ~ReduceAction();

    bool parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const override;

    std::string serialize() const override;

private:
    const GrammarSymbol* grammarSymbol;
    int productionNumber;

    const ParsingTable* parsingTable;
};

}

#endif /* REDUCEACTION_H_ */
