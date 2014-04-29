#ifndef _FINITE_AUTOMATON_SCANNER_H_
#define _FINITE_AUTOMATON_SCANNER_H_

#include <memory>

#include "Scanner.h"

class StateMachine;

class StateMachineFactory;

class FiniteAutomatonScanner: public Scanner {
public:

	FiniteAutomatonScanner(std::unique_ptr<StateMachineFactory> stateMachineFactory);
	virtual ~FiniteAutomatonScanner();

	Token scan(TranslationUnit& translationUnit);

private:
	std::unique_ptr<StateMachine> automaton;
};

#endif // _FINITE_AUTOMATON_SCANNER_H_
