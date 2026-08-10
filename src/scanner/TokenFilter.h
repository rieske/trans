#ifndef SCANNER_TOKEN_FILTER_H_
#define SCANNER_TOKEN_FILTER_H_

#include "scanner/Token.h"

#include <deque>
#include <functional>

namespace scanner {

// Post-FA token rewrite: C lex phases (wide prefixes, adjacent string concat)
// and gcc -E dialect (GNU spellings, dropped markers, attributes/asm).
// Not the finite automaton and not TokenStream typedef reclassify.
class TokenFilter {
public:
    explicit TokenFilter(std::function<Token()> raw, bool gnuExtensions = true);

    Token nextToken();

private:
    Token nextRaw();
    Token nextBaseFiltered();
    Token finishStringToken(const Token& first);
    void skipBalancedParenGroup();
    void pushFront(Token t);
    static bool isStringToken(const Token& t);

    std::function<Token()> raw_;
    std::deque<Token> pending_;
    bool gnuExtensions_;
};

} // namespace scanner

#endif
