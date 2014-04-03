#ifndef _ACTION_H_
#define _ACTION_H_

#include <fstream>
#include "rule.h"
#include "token.h"

using std::ofstream;

/**
 * @author Vaidotas Valuckas
 **/

class Action
{
    public:
        Action(char t, long s);
        ~Action();

        char which() const;
        long getState() const;
        Rule *getReduction() const;

        void setReduction(Rule *r);
        
        void setExpected(string e);
        string getExpected() const;

        int error(Token *) const;
        unsigned getForge() const;
        void setForge(unsigned forge);

        void print() const;
        void log(ofstream &out) const;
        void output(ofstream &out) const;

    private:
        char type;
        long state;

        Rule *reduction;

        string *expected;
        unsigned forge_token;
};

#endif // _ACTION_H_
