#ifndef _ACTION_H_
#define _ACTION_H_

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "TokenStream.h"
#include "SyntaxTreeBuilder.h"

namespace parser {

class Grammar;
class Production;
using parse_state = size_t;

// Value-type LR action cell (shift / reduce / accept / error).
// Table generation and error reporting. The product loop does not execute these.
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
    static Action reduce(const Production& production);
    static Action accept();
    static Action error(parse_state state,
            std::shared_ptr<const std::vector<int>> candidateSymbols,
            const Grammar* grammar);

    Kind kind() const noexcept { return kind_; }
    parse_state shiftState() const;
    int productionId() const;

    void reportError(TokenStream& tokenStream, SyntaxTreeBuilder& syntaxTreeBuilder) const;

    std::string toString() const;
    bool equals(const Action& other) const;
    bool isCorrective() const noexcept { return kind_ == Kind::Reduce; }

private:
    Kind kind_ { Kind::Accept };
    parse_state state_ { 0 };
    const Production* production_ { nullptr };
    std::shared_ptr<const std::vector<int>> candidateSymbols_;
    const Grammar* grammar_ { nullptr };
};

} // namespace parser

#endif // _ACTION_H_
