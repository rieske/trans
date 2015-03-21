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

namespace code_generator {

class InstructionSet;

class StackMachine {
public:
    StackMachine(std::ostream* ostream, std::unique_ptr<InstructionSet> instructions);
    virtual ~StackMachine() = default;

    void startProcedure(std::string procedureName);
    void endProcedure();

    void label(std::string name) const;
    void jump(JumpCondition jumpCondition, std::string label);

    void allocateStack(std::vector<Value> values);
    void deallocateStack();

    void storeGeneralPurposeRegisterValues();
    void freeIOregister();

    void callInputProcedure();
    void callOutputProcedure();
    void storeIOregisterIn(std::string symbolName);
    void assignIOregisterTo(std::string symbolName);

    void compare(std::string leftSymbolName, std::string rightSymbolName);
    void zeroCompare(std::string symbolName);

    void addressOf(std::string operandName, std::string resultName);
    void dereference(std::string operandName, std::string lvalueName, std::string resultName);

    void unaryMinus(std::string operandName, std::string resultName);

    void assign(std::string operandName, std::string resultName);
    void assignConstant(std::string constant, std::string resultName);
    void lvalueAssign(std::string constant, std::string resultName);

private:
    void storeRegisterValue(Register& reg);
    void emptyGeneralPurposeRegisters();

    void storeInMemory(Value& symbol);

    int memoryOffset(const Value& symbol) const;
    const Register& memoryBaseRegister(const Value& symbol) const;

    Register& getRegister();
    Register& getRegisterExcluding(std::string registerName);
    Register& assignRegisterTo(Value& symbol);

    std::ostream* ostream;
    std::unique_ptr<InstructionSet> instructions;

    Register stackPointer;
    Register basePointer;
    std::map<std::string, Register> generalPurposeRegisters;
    std::string ioRegisterName;

    std::map<std::string, Value> scopeValues;

    bool main { false };
};

} /* namespace code_generator */

#endif /* STACKMACHINE_H_ */
