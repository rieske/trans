#ifndef _QUADRUPLE_H_
#define _QUADRUPLE_H_

#include <iostream>
#include <map>
#include <string>

#include "../semantic_analyzer/ValueEntry.h"

namespace semantic_analyzer {
class FunctionEntry;
class LabelEntry;
} /* namespace semantic_analyzer */

namespace code_generator {

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
    Quadruple(unsigned op, semantic_analyzer::ValueEntry *arg1, semantic_analyzer::ValueEntry *arg2, semantic_analyzer::ValueEntry *res);
    Quadruple(std::string val, semantic_analyzer::ValueEntry *res);
    Quadruple(unsigned op, semantic_analyzer::LabelEntry *label);
    Quadruple(unsigned op, semantic_analyzer::FunctionEntry* function);
    Quadruple(unsigned op, std::map<std::string, semantic_analyzer::ValueEntry> symbols, std::map<std::string, semantic_analyzer::ValueEntry> arguments);

    void output(std::ostream &of) const;

    unsigned getOp() const;
    semantic_analyzer::ValueEntry *getArg1() const;
    semantic_analyzer::ValueEntry *getArg2() const;
    semantic_analyzer::ValueEntry *getRes() const;

    std::string getConstant() const;

    semantic_analyzer::LabelEntry* getLabel() const;
    semantic_analyzer::FunctionEntry* getFunction() const;

    std::map<std::string, semantic_analyzer::ValueEntry> getSymbols() const;
    std::map<std::string, semantic_analyzer::ValueEntry> getArguments() const;
    private:
    unsigned op;
    semantic_analyzer::ValueEntry* arg1 { nullptr };
    semantic_analyzer::ValueEntry* arg2 { nullptr };
    semantic_analyzer::ValueEntry* res { nullptr };
    std::string constant;
    semantic_analyzer::LabelEntry* label { nullptr };
    semantic_analyzer::FunctionEntry* function { nullptr };

    std::map<std::string, semantic_analyzer::ValueEntry> symbols;
    std::map<std::string, semantic_analyzer::ValueEntry> arguments;
};

}

#endif // _QUADRUPLE_H_
