#ifndef _QUADRUPLE_H_
#define _QUADRUPLE_H_

#include <string>
#include <iostream>
#include <fstream>
#include "symbol_entry.h"

using std::string;
using std::ostream;

/**
 * @author Vaidotas Valuckas
 * Tetrados klasė
 * (op, arg1, arg2, res)
 **/

enum oper
{
    ASSIGN,
    ADD, SUB,
    INC, DEC,
    MUL, DIV, MOD,
    AND, OR, XOR,
    NOT,
    PARAM, CALL, RETURN, RETRIEVE,
    GOTO,
    ADDR, DEREF, DEREF_LVAL, UMINUS,
    SHR, SHL,
    CMP,
    JE, JNE, JA, JB, JAE, JBE,
    LABEL,
    INDEX, INDEX_ADDR,
    PROC, ENDPROC,
    IN, OUT,
    SCOPE, ENDSCOPE
};

class Quadruple
{
    public:
        Quadruple(unsigned op, SymbolEntry *arg1, SymbolEntry *arg2, SymbolEntry *res);
        Quadruple(string val, SymbolEntry *res);

        void output(ostream &of) const;

        void setArg1(SymbolEntry *arg1);
        void setOp (unsigned op);

        unsigned    getOp() const;
        SymbolEntry *getArg1() const;
        SymbolEntry *getArg2() const;
        SymbolEntry *getRes() const;

        string getConstant() const;

    private:
        unsigned op;
        SymbolEntry *arg1;
        SymbolEntry *arg2;
        SymbolEntry *res;
        string constant;
};

#endif // _QUADRUPLE_H_
