#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#define START_SYMBOL "<__start__>"
#define END_SYMBOL "'$end$'"

#include <map>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include "rule.h"
#include "set_of_items.h"

using std::map;
using std::fstream;

/**
 * @author Vaidotas Valuckas
 * Gramatikos klasė
 **/

class Grammar
{
    public:
        Grammar();
        Grammar(const char *bnf);
        ~Grammar();

        Rule        *getRule()  const;
        Grammar     *getNext()  const;     

        const vector<string *> *getNonterminals() const;
        const map<unsigned, string *> *getTerminals() const;

        void        addRule(Rule *g);
        void        addTerminal(unsigned, string *);
        void        addNonterminal(string *);

        Set_of_items *closure(Set_of_items *I) const;
        Set_of_items *go_to(Set_of_items *I, string *X) const;

        vector<Set_of_items *> *canonical_collection() const;

        bool is_terminal(string *str) const;
        bool is_nonterminal(string *str) const;

        void        print()     const;
        void        printAddr()     const;

        void        output(ofstream &out) const;
        void        log(ofstream &out) const;

    private:
        Grammar(Rule *r);

        void    parseBnf(const char *bnf);
        void    fillSymbols();
        void    fillFirst();
        bool    addFirst(string *nonterm, string *first);
        bool    addFirstRow(string *dest, string *src);

        bool contains(vector<string *> *vect, string *str) const;

        void    print_terminals()   const;
        void    print_nonterminals() const;
        void    print_first_table() const;

        void    log_terminals(ofstream &out)   const;
        void    log_nonterminals(ofstream &out) const;
        void    log_first_table(ofstream &out) const;

        // ****************************************************

        Rule        *rule;
        Grammar     *next;

        static string *start_symbol;
        static string *end_symbol;

        static vector<string *> *symbols;
        static vector<string *> *nonterminals;
        static map<unsigned, string *> *terminals;

        static map<string *, vector<string *> * >    *first_table;
};

#endif // _GRAMMAR_H_
