#include "FiniteAutomatonScanner.h"

#include "../driver/TranslationUnit.h"
#include "StateMachine.h"
#include "StateMachineFactory.h"
#include "Token.h"

bool FiniteAutomatonScanner::log = false;

FiniteAutomatonScanner::FiniteAutomatonScanner(std::unique_ptr<StateMachineFactory> stateMachineFactory) {
	automaton = stateMachineFactory->createAutomaton();
}

FiniteAutomatonScanner::~FiniteAutomatonScanner() {
}

/*void FiniteAutomatonScanner::print_states() const {
 map<string, State*>::const_iterator it;           // išspausdinam sudarytą baigtinį automatą
 for (it = m_state.begin(); it != m_state.end(); ++it) {
 fprintf(logfile, "%s:%d\n", it->first.c_str(), it->second->getTokenId());
 it->second->listing(logfile);
 }

 if (!m_keywords.empty()) {
 fprintf(logfile, "\nKeywords:\n");
 map<string, unsigned int>::const_iterator kw_it;        // išspausdinam raktinių žodžių lentelę
 for (kw_it = m_keywords.begin(); kw_it != m_keywords.end(); ++kw_it)
 fprintf(logfile, "%s:%d\n", kw_it->first.c_str(), kw_it->second);
 }
 }*/

Token FiniteAutomatonScanner::scan(TranslationUnit& translationUnit) {
	char currentCharacter;
	do {
		currentCharacter = translationUnit.getNextCharacter();
		automaton->updateState(currentCharacter);
	} while (!automaton->isAtFinalState() && currentCharacter != '\0');
	return automaton->getCurrentToken();
}

void FiniteAutomatonScanner::set_logging(const char *lf) {
	log = true;
}
