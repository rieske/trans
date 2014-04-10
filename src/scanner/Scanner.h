#ifndef _SCANNER_H_
#define _SCANNER_H_

#include <map>
#include <string>
#include <string.h>
#include "State.h"
#include "Token.h"

using std::string;
using std::map;

/**
 * Skanerio klasė
 * sukūrimui reikalingas konfigūracinio failo vardas,
 * pagrindinė funkcija - grąžinti kitą leksemą
 **/

class Scanner
{
    public:
        Scanner(const char *conf);
        ~Scanner();

        Token           *scan();         // pagrindinė skanerio funkcija - grąžinti kitą leksemą

        static void     set_logging(const char *);

        void            print_states()  const;

    private:
        int             init(const char *source);

        State           *update_state(char c);

        FILE            *source;         // skenuojamas failas
        char            *buffer;         // saugo skaitomą eilutę

        FILE            *lex_src;        // baigtinio automato konfigūracija

        unsigned long   b_index;         // einamojo simbolio indeksas
        string          token;           // kaupiama leksema
        unsigned long   line;            // einamoji eilutė

        map<string, State *>        m_state;        // čia saugomos visos automato būsenos
        map<string, unsigned int>   m_keywords;     // rezervuotų žodžių lentelė <žodis, kodas>
        string          start_state;
        string          final_state;
        State           *current_state;             // einamoji būsena
        string          current_state_str;

        static bool     log;
        static FILE     *logfile;
};

#endif // _SCANNER_H_
