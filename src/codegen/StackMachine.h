#ifndef STACKMACHINE_H_
#define STACKMACHINE_H_

#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include <stack>

#include "InstructionSet.h"
#include "Amd64Registers.h"
#include "quadruples/Jump.h"
#include "Value.h"
#include "Assembly.h"
#include "instructions/Instructions.h"

namespace codegen {

class StackMachine {
public:
    StackMachine(std::ostream* ostream, std::unique_ptr<InstructionSet> instructions, std::unique_ptr<Amd64Registers> registers);
    StackMachine(const StackMachine&) = delete;
    StackMachine(StackMachine&&) = default;
    virtual ~StackMachine() = default;

    StackMachine& operator=(const StackMachine&) = delete;
    StackMachine& operator=(StackMachine&&) = default;

    void generatePreamble();

    void startProcedure(std::string procedureName, std::vector<Value> values, std::vector<Value> arguments);
    void endProcedure();

    void label(std::string name);
    void jump(JumpCondition jumpCondition, std::string label);

    void callInputProcedure(std::string symbolName);
    void callOutputProcedure(std::string symbolName);

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
    void returnFromProcedure(std::string returnSymbolName = "");
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

    void saveCallerSavedRegisters();
    void popCallerSavedRegisters();

    void pushCalleeSavedRegisters();
    void popCalleeSavedRegisters();

    void pushDirtyRegisters(std::vector<Register*> source, std::vector<Register*>& destination);

    void pushRegisters(std::vector<Register*> source, std::vector<Register*>& destination);
    void popRegisters(std::vector<Register*> registers);

    void pushRegister(Register& reg, std::vector<Register*>& registers);

    void storeInMemory(Value& symbol);

    int memoryOffset(const Value& symbol) const;
    const Register& memoryBaseRegister(const Value& symbol) const;

    Register& get64BitRegister();
    Register& get64BitRegisterExcluding(Register& registerToExclude);
    Register& assignRegisterTo(Value& symbol);
    void assignRegisterToSymbol(Register& reg, Value& symbol);
    Register& assignRegisterExcluding(Value& symbol, Register& registerToExclude);

    std::vector<std::shared_ptr<Instruction>> instructions;
    Assembly assembly;
    std::unique_ptr<InstructionSet> instructionSet;

    std::unique_ptr<Amd64Registers> registers;
    std::vector<Register*> callerSavedRegisters;
    std::vector<Register*> calleeSavedRegisters;

    std::map<std::string, Value> scopeValues;
    std::vector<Value*> integerArguments;
    std::vector<Value*> stackArguments;

    int localVariableStackSize { 0 };
};

} /* namespace codegen */

#endif /* STACKMACHINE_H_ */
