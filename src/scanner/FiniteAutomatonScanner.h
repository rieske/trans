#ifndef _FINITE_AUTOMATON_SCANNER_H_
#define _FINITE_AUTOMATON_SCANNER_H_

#include <memory>

#include "Scanner.h"

class TranslationUnit;
class StateMachine;
class StateMachineFactory;

class FiniteAutomatonScanner: public Scanner {
public:

	FiniteAutomatonScanner(TranslationUnit* translationUnit, StateMachineFactory* stateMachineFactory);
	virtual ~FiniteAutomatonScanner();

	Token nextToken() override;
	Token currentToken() override;
private:
	std::unique_ptr<TranslationUnit> translationUnit;
	std::unique_ptr<StateMachine> automaton;

	std::unique_ptr<Token> _currentToken;
};

#endif // _FINITE_AUTOMATON_SCANNER_H_
