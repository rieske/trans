#ifndef _CODE_GENERATOR_H_
#define _CODE_GENERATOR_H_

#include "quadruple.h"
#include "symbol_table.h"
#include "register.h"

#include <fstream>
#include <string>

using std::ofstream;
using std::string;

/**
 * @author Vaidotas Valuckas
 * asemblerinio kodo generatoriaus klasė
 * sugeneruoja NASM asemblerio kodą iš tetradų vektoriaus
 **/

class CodeGenerator
{
    public:
        CodeGenerator(const char *src);
        ~CodeGenerator();

        int generateCode(vector<Quadruple *> code, SymbolTable *s_table);

        int assemble();
        int link();

    private:
        Register *getReg();
        Register *getReg(Register *reg);
        Register *getRegByName(string regName) const;
        
        void assign(SymbolEntry *arg, SymbolEntry *place, string constant);
        void output(SymbolEntry *arg1);
        void add(SymbolEntry *arg1, SymbolEntry *arg2, SymbolEntry *res);
        void sub(SymbolEntry *arg1, SymbolEntry *arg2, SymbolEntry *res);
        void inc(SymbolEntry *arg1);
        void dec(SymbolEntry *arg1);
        void mul(SymbolEntry *arg1, SymbolEntry *arg2, SymbolEntry *res);
        void div(SymbolEntry *arg1, SymbolEntry *arg2, SymbolEntry *res);
        void mod(SymbolEntry *arg1, SymbolEntry *arg2, SymbolEntry *res);
        void and_(SymbolEntry *arg1, SymbolEntry *arg2, SymbolEntry *res);
        void or_(SymbolEntry *arg1, SymbolEntry *arg2, SymbolEntry *res);
        void xor_(SymbolEntry *arg1, SymbolEntry *arg2, SymbolEntry *res);
        void addr(SymbolEntry *arg1, SymbolEntry *res);
        void deref(SymbolEntry *arg1, SymbolEntry *res);
        void deref_lval(SymbolEntry *arg1, SymbolEntry *res);
        void uminus(SymbolEntry *arg1, SymbolEntry *res);
        void shr(SymbolEntry *arg1, SymbolEntry *res);
        void shl(SymbolEntry *arg1, SymbolEntry *res);
        void input(SymbolEntry *arg1);
        void cmp(SymbolEntry *arg1, SymbolEntry *arg2);
        void ret(SymbolEntry *arg);
        void call(SymbolEntry *arg);
        void param(SymbolEntry *arg1);
        void retrieve(SymbolEntry *arg);

        //vector<Quadruple *> code;
        //SymbolTable *s_table;
        bool main;
        unsigned paramOffset;
        vector<SymbolEntry *> params;

        string *fname;
        ofstream outfile;

        Register *eax;
        Register *ebx;
        Register *ecx;
        Register *edx;
};

#endif // _CODE_GENERATOR_H_
