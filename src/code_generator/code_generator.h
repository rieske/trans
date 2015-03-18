#ifndef _CODE_GENERATOR_H_
#define _CODE_GENERATOR_H_

#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "quadruple.h"
#include "Value.h"

namespace code_generator {
class Function;
} /* namespace code_generator */

namespace semantic_analyzer {
class ValueEntry;
} /* namespace semantic_analyzer */

/**
 * generates code for the NASM assembler given a quadruple vector
 **/

namespace code_generator {

class Register;

class CodeGenerator {
public:
    CodeGenerator(const char *src);
    ~CodeGenerator();

    int generateCode(std::vector<Quadruple_deprecated> code);

    int assemble();
    int link();

private:
    Register *getReg();
    Register *getReg(Register *reg);
    Register *getRegByName(std::string regName) const;

    void assign(Value *arg, Value *place, std::string constant);
    void output(Value *arg1);
    void add(Value *arg1, Value *arg2, Value *res);
    void sub(Value *arg1, Value *arg2, Value *res);
    void inc(Value *arg1);
    void dec(Value *arg1);
    void mul(Value *arg1, Value *arg2, Value *res);
    void div(Value *arg1, Value *arg2, Value *res);
    void mod(Value *arg1, Value *arg2, Value *res);
    void and_(Value *arg1, Value *arg2, Value *res);
    void or_(Value *arg1, Value *arg2, Value *res);
    void xor_(Value *arg1, Value *arg2, Value *res);
    void addr(Value *arg1, Value *res);
    void deref(Value *arg1, Value *res);
    void deref_lval(Value *arg1, Value *res);
    void uminus(Value *arg1, Value *res);
    void shr(Value *arg1, Value *res);
    void shl(Value *arg1, Value *res);
    void input(Value *arg1);
    void cmp(Value *arg1, Value *arg2);
    void ret(Value *arg);
    void call(semantic_analyzer::FunctionEntry *arg);
    void param(Value *arg1);
    void retrieve(Value *arg);

    void store(Value* symbol);

    Value* getCurrentScopeValue(semantic_analyzer::ValueEntry* optionalValue);
    void endScope();

    bool main;
    unsigned paramOffset;
    std::vector<Value*> params;

    std::string *fname;
    std::ofstream outfile;

    Register *eax;
    Register *ebx;
    Register *ecx;
    Register *edx;

    std::map<std::string, Value> currentScopeValues;
};

std::string getOffsetRegister(Value* symbol);
std::string getStoragePlace(Value* symbol);

int computeOffset(Value* symbol);

}

#endif // _CODE_GENERATOR_H_
