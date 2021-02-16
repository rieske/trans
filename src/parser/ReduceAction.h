#ifndef REDUCEACTION_H_
#define REDUCEACTION_H_

#include "Action.h"

namespace parser {

class ReduceAction: public Action {
public:
    ReduceAction(const Production& production, const ParsingTable* parsingTable);
    virtual ~ReduceAction();

    bool parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const override;

    std::string serialize() const override;

private:
    Production production;

    const ParsingTable* parsingTable;
};

} // namespace parser

#endif // REDUCEACTION_H_
