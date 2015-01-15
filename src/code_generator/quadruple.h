#ifndef _QUADRUPLE_H_
#define _QUADRUPLE_H_

#include <iostream>
#include <map>
#include <string>

#include "ValueEntry.h"

namespace code_generator {

class FunctionEntry;
class LabelEntry;
class ValueEntry;

/**
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

class Quadruple {
public:
    Quadruple(unsigned op, ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res);
    Quadruple(std::string val, ValueEntry *res);
    Quadruple(unsigned op, LabelEntry *label);
    Quadruple(unsigned op, FunctionEntry* function);
    Quadruple(unsigned op, std::map<std::string, ValueEntry> symbols, std::map<std::string, ValueEntry> arguments);

    void output(std::ostream &of) const;

    void setArg1(ValueEntry *arg1);
    void setOp (unsigned op);

    unsigned    getOp() const;
    ValueEntry *getArg1() const;
    ValueEntry *getArg2() const;
    ValueEntry *getRes() const;

    std::string getConstant() const;

    LabelEntry* getLabel() const;
    FunctionEntry* getFunction() const;

    std::map<std::string, ValueEntry> getSymbols() const;
    std::map<std::string, ValueEntry> getArguments() const;
private:
    unsigned op;
    ValueEntry *arg1 {nullptr};
    ValueEntry *arg2 {nullptr};
    ValueEntry* res;
    std::string constant;
    LabelEntry* label;
    FunctionEntry* function;

    std::map<std::string, ValueEntry> symbols;
    std::map<std::string, ValueEntry> arguments;
};

}

#endif // _QUADRUPLE_H_
