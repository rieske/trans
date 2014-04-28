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

	static void set_logging(const char *);

private:
	std::unique_ptr<StateMachine> automaton;

	void print_states() const;

	static bool log;
};

#endif // _FINITE_AUTOMATON_SCANNER_H_
