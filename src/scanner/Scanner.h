#ifndef SCANNER_H_
#define SCANNER_H_

#include "scanner/FiniteAutomaton.h"
#include "scanner/Token.h"
#include "translation_unit/TranslationUnit.h"

#include <memory>

namespace scanner {

class Scanner {
public:
    Scanner(std::string fileName, FiniteAutomaton* stateMachine);

    Token nextToken();

private:
    TranslationUnit translationUnit;
    std::unique_ptr<FiniteAutomaton> automaton;
};

} // namespace scanner

#endif // SCANNER_H_
