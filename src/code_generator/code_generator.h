#ifndef _CODE_GENERATOR_H_
#define _CODE_GENERATOR_H_

#include <fstream>
#include <string>
#include <vector>

#include "quadruple.h"
#include "ValueEntry.h"
#include "FunctionEntry.h"

/**
 * generates code for the NASM assembler given a quadruple vector
 **/

namespace code_generator {

class Register;
class SymbolTable;

class CodeGenerator {
public:
    CodeGenerator(const char *src);
    ~CodeGenerator();

    int generateCode(std::vector<Quadruple> code);

    int assemble();
    int link();

private:
    Register *getReg();
    Register *getReg(Register *reg);
    Register *getRegByName(std::string regName) const;

    void assign(ValueEntry *arg, ValueEntry *place, std::string constant);
    void output(ValueEntry *arg1);
    void add(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res);
    void sub(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res);
    void inc(ValueEntry *arg1);
    void dec(ValueEntry *arg1);
    void mul(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res);
    void div(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res);
    void mod(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res);
    void and_(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res);
    void or_(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res);
    void xor_(ValueEntry *arg1, ValueEntry *arg2, ValueEntry *res);
    void addr(ValueEntry *arg1, ValueEntry *res);
    void deref(ValueEntry *arg1, ValueEntry *res);
    void deref_lval(ValueEntry *arg1, ValueEntry *res);
    void uminus(ValueEntry *arg1, ValueEntry *res);
    void shr(ValueEntry *arg1, ValueEntry *res);
    void shl(ValueEntry *arg1, ValueEntry *res);
    void input(ValueEntry *arg1);
    void cmp(ValueEntry *arg1, ValueEntry *arg2);
    void ret(ValueEntry *arg);
    void call(FunctionEntry *arg);
    void param(ValueEntry *arg1);
    void retrieve(ValueEntry *arg);

    bool main;
    unsigned paramOffset;
    std::vector<ValueEntry*> params;

    std::string *fname;
    std::ofstream outfile;

    Register *eax;
    Register *ebx;
    Register *ecx;
    Register *edx;
};

std::string getOffsetRegister(ValueEntry* symbol);
std::string getStoragePlace(ValueEntry* symbol);
std::string store(ValueEntry* symbol);
int computeOffset(ValueEntry* symbol);


}

#endif // _CODE_GENERATOR_H_
