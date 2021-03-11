#ifndef _ACTION_H_
#define _ACTION_H_

#include <memory>
#include <stack>
#include <string>

#include "ParsingTable.h"
#include "TokenStream.h"

#include "SyntaxTreeBuilder.h"

namespace parser {

class Action {
public:
    virtual ~Action();

    virtual bool parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const = 0;

    virtual std::string serialize() const = 0;

    static std::unique_ptr<Action> deserialize(std::string serializedAction, const ParsingTable& parsingTable, const Grammar& grammar);
};

} // namespace parser

#endif // _ACTION_H_
