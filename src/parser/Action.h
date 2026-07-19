#ifndef _ACTION_H_
#define _ACTION_H_

#include <cstddef>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "TokenStream.h"
#include "SyntaxTreeBuilder.h"

namespace parser {

class ParsingTable;
class Grammar;
class Production;
using parse_state = size_t;

// Value-type LR action cell (shift / reduce / accept / error).
// Replaces the former virtual Action hierarchy + unique_ptr heap cells.
class Action {
public:
    enum class Kind : char {
        Shift = 's',
        Reduce = 'r',
        Accept = 'a',
        Error = 'e',
    };

    Action() = default;

    static Action shift(parse_state state);
    static Action reduce(const Production& production, const ParsingTable* parsingTable);
    static Action accept();
    static Action error(parse_state state,
            std::shared_ptr<const std::vector<int>> candidateSymbols,
            const Grammar* grammar);

    Kind kind() const noexcept { return kind_; }

    // Returns true when the parse is finished (accept or error).
    bool parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream,
            SyntaxTreeBuilder& syntaxTreeBuilder) const;

    std::string serialize() const;
    bool equals(const Action& other) const;
    bool isCorrective() const noexcept { return kind_ == Kind::Reduce; }

    static Action deserialize(const std::string& serializedAction,
            const ParsingTable& parsingTable, const Grammar& grammar);

private:
    Kind kind_ { Kind::Accept };
    parse_state state_ { 0 };
    const Production* production_ { nullptr };
    const ParsingTable* parsingTable_ { nullptr };
    std::shared_ptr<const std::vector<int>> candidateSymbols_;
    const Grammar* grammar_ { nullptr };
};

} // namespace parser

#endif // _ACTION_H_
