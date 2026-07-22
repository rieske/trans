#ifndef STACKMACHINE_H_
#define STACKMACHINE_H_

#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "Address.h"
#include "InstructionSet.h"
#include "Amd64Registers.h"
#include "GlobalVariable.h"
#include "quadruples/Jump.h"
#include "Value.h"
#include "Assembly.h"

namespace codegen {

class StackMachine {
public:
    StackMachine(std::ostream* ostream, std::unique_ptr<InstructionSet> instructions, std::unique_ptr<Amd64Registers> registers);
    StackMachine(const StackMachine&) = delete;
    StackMachine(StackMachine&&) = default;
    virtual ~StackMachine() = default;

    StackMachine& operator=(const StackMachine&) = delete;
    StackMachine& operator=(StackMachine&&) = default;

    void generatePreamble(const std::map<std::string, std::string>& constants,
            const std::vector<GlobalVariable>& globalVariables);

    void startProcedure(std::string procedureName, std::vector<Value> values, std::vector<Value> arguments);
    void endProcedure();

    void label(std::string name);
    void jump(JumpCondition jumpCondition, std::string label);

    void compare(std::string leftSymbolName, std::string rightSymbolName);
    void zeroCompare(std::string symbolName);

    void addressOf(std::string operandName, std::string resultName);
    void dereference(std::string operandName, std::string lvalueName, std::string resultName);

    void unaryMinus(std::string operandName, std::string resultName);
    void unaryNot(std::string operandName, std::string resultName);

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

    void shl(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void shr(std::string leftOperandName, std::string rightOperandName, std::string resultName);

    void setScope(std::vector<Value> variables);

private:
    void shiftBy(std::string leftOperandName, std::string rightOperandName, std::string resultName,
            std::string (InstructionSet::*emitShift)(const Register&) const);

    void pushProcedureArgument(Value& argument, int argumentOffset);

    void storeRegisterValue(Register& reg);
    void spillGeneralPurposeRegisters();
    void spillCallerSavedRegisters();
    void emptyGeneralPurposeRegisters();

    void pushCalleeSavedRegisters();
    void popCalleeSavedRegisters();

    void pushRegisters(std::vector<Register*> source, std::vector<Register*>& destination);
    void popRegisters(std::vector<Register*> registers);

    void pushRegister(Register& reg, std::vector<Register*>& registers);

    void storeInMemory(Value& symbol);

    // Resolve a symbol name to its Value: a per-frame local/argument, or a program-scoped global.
    Value& resolve(const std::string& name);

    // Prefer registered object homes; fall back to stack-pointer spill from Value::index.
    Address addressOf(const Value& symbol) const;
    Address spillSlotAddress(const Value& symbol) const;
    // True if the operand should be read/written in memory (global home or no register).
    bool residesInMemory(const Value& symbol) const;
    void registerFrameHome(const std::string& name, Address address);
    // Load into dest without reg.assign; required for globals (Address-only homes).
    void loadWithoutBinding(Value& symbol, Register& dest);
    MemoryOperand memoryOperand(const Address& address) const;
    MemoryOperand memoryOperand(const Value& symbol) const;
    void emitLoad(Value& symbol, Register& dest);
    void emitStore(Register& source, Value& symbol);
    void bindResult(Register& reg, Value& result);

    Register& get64BitRegister();
    Register& get64BitRegisterExcluding(Register& registerToExclude);
    Register& getCounterRegister();
    Register& assignRegisterTo(Value& symbol);
    void assignRegisterToSymbol(Register& reg, Value& symbol);
    Register& assignRegisterExcluding(Value& symbol, Register& registerToExclude);

    Assembly assembly;
    std::unique_ptr<InstructionSet> instructionSet;

    std::unique_ptr<Amd64Registers> registers;
    std::vector<Register*> calleeSavedRegisters;

    // Per-frame Values for resolve (temps and locals; may be register-resident).
    std::map<std::string, Value> scopeValues;
    // resolve() shells for globals only - not homes and never register-cached.
    std::map<std::string, Value> globals;
    // Object homes (Address); globals and frame slots.
    std::map<std::string, Address> globalHomes;
    std::map<std::string, Address> frameHomes;
    std::vector<Value*> integerArguments;
    std::vector<Value*> stackArguments;

    int localVariableStackSize { 0 };
};

} // namespace codegen

#endif // STACKMACHINE_H_
