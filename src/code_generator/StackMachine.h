#ifndef STACKMACHINE_H_
#define STACKMACHINE_H_

#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "quadruples/Jump.h"
#include "Register.h"
#include "Value.h"

namespace codegen {

class InstructionSet;

class StackMachine {
public:
    StackMachine(std::ostream* ostream, std::unique_ptr<InstructionSet> instructions);
    StackMachine(const StackMachine&) = delete;
    StackMachine(StackMachine&&) = default;
    virtual ~StackMachine() = default;

    StackMachine& operator=(const StackMachine&) = delete;
    StackMachine& operator=(StackMachine&&) = default;

    void startProcedure(std::string procedureName);
    void endProcedure();

    void label(std::string name) const;
    void jump(JumpCondition jumpCondition, std::string label);

    void allocateStack(std::vector<Value> values, std::vector<Value> arguments);
    void deallocateStack();

    void freeIOregister();

    void callInputProcedure(std::string symbolName);
    void callOutputProcedure(std::string symbolName);
    void storeIOregisterIn(std::string symbolName);

    void compare(std::string leftSymbolName, std::string rightSymbolName);
    void zeroCompare(std::string symbolName);

    void addressOf(std::string operandName, std::string resultName);
    void dereference(std::string operandName, std::string lvalueName, std::string resultName);

    void unaryMinus(std::string operandName, std::string resultName);

    void assign(std::string operandName, std::string resultName);
    void assignConstant(std::string constant, std::string resultName);
    void lvalueAssign(std::string operandName, std::string resultName);

    void procedureArgument(std::string argumentName);
    void callProcedure(std::string procedureName);
    void returnFromProcedure(std::string returnSymbolName);
    void retrieveProcedureReturnValue(std::string returnSymbolName);

    void xorCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void orCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void andCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName);

    void add(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void sub(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void mul(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void div(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void mod(std::string leftOperandName, std::string rightOperandName, std::string resultName);

    void inc(std::string operandName);
    void dec(std::string operandName);

private:
    void pushProcedureArgument(Value& argument, int argumentOffset);

    void storeRegisterValue(Register& reg);
    void storeGeneralPurposeRegisterValues();
    void emptyGeneralPurposeRegisters();

    void storeInMemory(Value& symbol);

    int memoryOffset(const Value& symbol) const;
    const Register& memoryBaseRegister(const Value& symbol) const;

    Register& get64BitRegister();
    Register& get64BitRegisterExcluding(Register& registerToExclude);
    Register& assignRegisterTo(Value& symbol);
    void assignRegisterToSymbol(Register& reg, Value& symbol);
    Register& assignRegisterExcluding(Value& symbol, Register& registerToExclude);

    std::ostream* ostream;
    std::unique_ptr<InstructionSet> instructions;

    Register rax { "rax" };
    Register rbx { "rbx" };
    Register rcx { "rcx" };
    Register rdx { "rdx" };
    Register rsi { "rsi" };
    Register rdi { "rdi" };

    Register r8 { "r8" };
    Register r9 { "r9" };
    Register r10 { "r10" };
    Register r11 { "r11" };
    Register r12 { "r12" };
    Register r13 { "r13" };
    Register r14 { "r14" };
    Register r15 { "r15" };

    Register stackPointer { "rsp" };
    Register basePointer { "rbp" };
    std::vector<Register*> generalPurposeRegisters { &rax, &rbx, &rcx, &rdx, &rdi, &rsi, &r8, &r9, &r10, &r11, &r12, &r13, &r14, &r15 };
    std::vector<Register*> integerArgumentRegisters { &rdi, &rsi, &rdx, &rcx, &r8, &r9 };
    Register* ioRegister { &rcx };
    Register* retrievalRegister { &rax };
    Register* multiplicationRegister { &rax };
    Register* remainderRegister { &rdx };

    std::map<std::string, Value> scopeValues;
    std::vector<std::string> argumentNames;

    bool main { false };
};

} /* namespace code_generator */

#endif /* STACKMACHINE_H_ */
