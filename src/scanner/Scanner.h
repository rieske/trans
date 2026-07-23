#ifndef SCANNER_H_
#define SCANNER_H_

#include "scanner/FiniteAutomaton.h"
#include "scanner/Token.h"
#include "translation_unit/TranslationUnit.h"

#include <deque>
#include <memory>

namespace scanner {

// Finite-automaton lexer with a thin token filter so post-gcc -E noise and
// C lexical conveniences never require a whole-file string rewrite layer:
// attributes/asm, GNU markers, # lines (TranslationUnit), wide string
// prefixes, adjacent string concatenation, _Bool, __int128 / _FloatN
// stand-ins, __func__.
class Scanner {
public:
    Scanner(std::string fileName, std::unique_ptr<FiniteAutomaton> stateMachine);

    Token nextToken();

private:
    Token nextTokenUnfiltered();
    Token nextTokenBaseFiltered();
    Token finishStringToken(const Token& first);
    void skipBalancedParenGroup();
    void pushFront(Token t);
    static bool isStringToken(const Token& t);

    TranslationUnit translationUnit;
    std::unique_ptr<FiniteAutomaton> automaton;
    std::deque<Token> pending;
};

} // namespace scanner

#endif // SCANNER_H_
