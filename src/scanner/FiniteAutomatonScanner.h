#ifndef _FINITE_AUTOMATON_SCANNER_H_
#define _FINITE_AUTOMATON_SCANNER_H_

#include <cstdio>
#include <map>
#include <memory>
#include <string>

#include "Scanner.h"
#include "State.h"

class StateMachine;

class StateMachineFactory;

using std::string;
using std::map;

class FiniteAutomatonScanner: public Scanner {
public:

	FiniteAutomatonScanner(std::unique_ptr<StateMachineFactory> stateMachineFactory);
	virtual ~FiniteAutomatonScanner();

	Token scan(TranslationUnit& translationUnit);

	static void set_logging(const char *);

private:
	std::unique_ptr<StateMachineFactory> stateMachineFactory;
	char lookaheadCharacter = '\0';

	void print_states() const;

	static bool log;
};

#endif // _FINITE_AUTOMATON_SCANNER_H_
