#ifndef _QUADRUPLE_H_
#define _QUADRUPLE_H_

#include <iostream>
#include <string>

namespace code_generator {

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
    Quadruple(unsigned op, int scopeSize);

    void output(std::ostream &of) const;

    void setArg1(ValueEntry *arg1);
    void setOp (unsigned op);

    unsigned    getOp() const;
    ValueEntry *getArg1() const;
    ValueEntry *getArg2() const;
    ValueEntry *getRes() const;

    std::string getConstant() const;

    LabelEntry* getLabel() const;

    int getScopeSize() const;
private:
    unsigned op;
    ValueEntry *arg1 {nullptr};
    ValueEntry *arg2 {nullptr};
    ValueEntry* res;
    std::string constant;
    LabelEntry* label;
    int scopeSize {0};
};

}

#endif // _QUADRUPLE_H_
