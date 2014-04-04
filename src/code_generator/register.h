#ifndef _REGISTER_H_
#define _REGISTER_H_

#include <string>
#include <stdlib.h>
#include "symbol_entry.h"

using std::string;

/**
 * @author Vaidotas Valuckas
 * procesoriaus registro klasė
 **/

enum regEnum
{
    EAX, EBX, ECX, EDX
};

class Register
{
    public:
        Register(regEnum w);

        string getName() const;
        bool isFree() const;
        void setValue(SymbolEntry *val);

        string free();

    private:
        regEnum which;
        string name;
        SymbolEntry *value;
};

#endif // _REGISTER_H_
