#ifndef _STATE_H_
#define _STATE_H_

#include <string>
#include <stdlib.h>
#include <vector>

using std::string;
using std::vector;

/**
 * @author Vaidotas Valuckas
 * Baigtinio automato būsenos klasė
 * saugo informaciją apie tai kur ir kokioms
 * sąlygoms esant iš jos galima patekti.
 **/

class State
{
    public:
        State();
        ~State();

        int         add_state(string next_state, string name);

        void        listing(FILE *)     const;

        string      get_next(char c)    const;  // grąžina būsenos vardą, į kurią turi patekti automatas esant sąlygai c
        int         getId()             const;  // leksemos kodas
        bool        need_lookup()       const;
        bool        is_comment()        const;
        bool        is_eol_comment()    const;

        void        setId(string id);
        void        setKeywordCheck();
        void        setIgnoreSpaces();
        void        setComment();
        void        setEolComment();

    private:
        string      same_state;             // simboliai, išlaikantys būseną
        vector<string> v_state;             // vardai būsenų, į kurias galima patekti
        vector<string> v_chars;             // simboliai, keičiantys būseną pagal indeksą sąraše

        int         id;                     // leksemos kodas

        bool        lookup;                 // ar reikia lygint su keywordais
        bool        ignoreSpaces;           // ar ignoruoti tarpus
        bool        comment;                // ar tai komentaras
        bool        eol_comment;            // komentaras iki eilutės galo
};

#endif // _STATE_H_
