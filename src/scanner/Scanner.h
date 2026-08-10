#ifndef SCANNER_H_
#define SCANNER_H_

#include "scanner/FiniteAutomaton.h"
#include "scanner/LexicalSession.h"
#include "scanner/Token.h"
#include "translation_unit/TranslationUnit.h"

#include <memory>

namespace scanner {

class Scanner {
public:
    Scanner(std::string fileName, std::unique_ptr<FiniteAutomaton> stateMachine, LexicalSession& session);

    Token nextToken();

    LexicalSession& session() { return session_; }
    const LexicalSession& session() const { return session_; }

private:
    TranslationUnit translationUnit;
    std::unique_ptr<FiniteAutomaton> automaton;
    LexicalSession& session_;
};

} // namespace scanner

#endif // SCANNER_H_
