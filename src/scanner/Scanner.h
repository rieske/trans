#ifndef SCANNER_H_
#define SCANNER_H_

#include "scanner/FiniteAutomaton.h"
#include "scanner/LexicalSession.h"
#include "scanner/Token.h"
#include "translation_unit/TranslationUnit.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace scanner {

class LexError : public std::runtime_error {
public:
    LexError(translation_unit::Context where, std::string message) :
            std::runtime_error { std::move(message) },
            where { std::move(where) } {
    }

    translation_unit::Context where;
};

class Scanner {
public:
    Scanner(std::string fileName, std::unique_ptr<FiniteAutomaton> stateMachine, LexicalSession& session);

    Token nextToken();

    LexicalSession& session() { return session_; }

private:
    TranslationUnit translationUnit;
    std::unique_ptr<FiniteAutomaton> automaton;
    LexicalSession& session_;
};

} // namespace scanner

#endif // SCANNER_H_
