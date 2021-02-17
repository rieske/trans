#ifndef _FINITE_AUTOMATON_SCANNER_H_
#define _FINITE_AUTOMATON_SCANNER_H_

#include <memory>

#include "Scanner.h"
#include "scanner/FiniteAutomaton.h"
#include "translation_unit/TranslationUnit.h"

namespace scanner {

class FiniteAutomatonScanner: public Scanner {
public:
    FiniteAutomatonScanner(TranslationUnit* translationUnit, FiniteAutomaton* stateMachine);
    virtual ~FiniteAutomatonScanner();

    Token nextToken() override;
private:
    std::unique_ptr<TranslationUnit> translationUnit;
    std::unique_ptr<FiniteAutomaton> automaton;
};

} // namespace scanner

#endif // _FINITE_AUTOMATON_SCANNER_H_
