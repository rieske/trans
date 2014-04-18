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
	FiniteAutomatonScanner(const char *conf);
	virtual ~FiniteAutomatonScanner();

	Token scan(TranslationUnit& translationUnit);

	Token *scan();         // pagrindinė skanerio funkcija - grąžinti kitą leksemą

	static void set_logging(const char *);

private:
	std::unique_ptr<StateMachineFactory> stateMachineFactory;
	char lookaheadCharacter = '\0';


	int init();

	void print_states() const;

	State *update_state(char c);

	FILE *source;         // skenuojamas failas
	char *buffer;         // saugo skaitomą eilutę

	FILE *lex_src;        // baigtinio automato konfigūracija

	unsigned long b_index;         // einamojo simbolio indeksas
	string token;           // kaupiama leksema

	map<string, State *> m_state;        // čia saugomos visos automato būsenos
	map<string, unsigned int> m_keywords;     // rezervuotų žodžių lentelė <žodis, kodas>
	string start_state;
	string final_state;

	unsigned long line;            // einamoji eilutė
	State *current_state;             // einamoji būsena
	string current_state_str;

	static bool log;
	static FILE *logfile;
};

#endif // _FINITE_AUTOMATON_SCANNER_H_
