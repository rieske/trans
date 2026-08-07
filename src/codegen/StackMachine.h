#ifndef STACKMACHINE_H_
#define STACKMACHINE_H_

#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>

#include "Address.h"
#include "InstructionSet.h"
#include "Amd64Registers.h"
#include "GlobalVariable.h"
#include "JumpCondition.h"
#include "SysVCallConv.h"
#include "Value.h"
#include "symbols/AddressPlan.h"
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
            const std::vector<GlobalVariable>& globalVariables,
            const std::vector<std::string>& externalFunctions = {},
            const std::vector<std::string>& definedFunctions = {});

    void startProcedure(std::string procedureName, std::vector<Value> values, std::vector<Value> arguments,
            bool variadic = false, bool memoryReturn = false);
    void endProcedure();

    // After each body quadruple: drop dead expression temps from registers without
    // spilling them (their stack slots may already be reused by later temps).
    void finishInstruction();

    void label(std::string name);
    void jump(JumpCondition jumpCondition, std::string label);

    void compare(std::string leftSymbolName, std::string rightSymbolName);
    void zeroCompare(std::string symbolName);

    void addressOf(std::string operandName, std::string resultName);
    void functionAddress(std::string functionName, std::string resultName);
    // Address of a pool/data label (string constants); always local lea, not GOT.
    void assignLabelAddress(std::string label, std::string resultName);
    void dereference(std::string operandName, std::string lvalueName, std::string resultName, int accessSizeBytes = 8,
            bool isSigned = true);

    void unaryMinus(std::string operandName, std::string resultName);
    void bitwiseNot(std::string operandName, std::string resultName);

    void assign(std::string operandName, std::string resultName);
    void assignConstant(std::string constant, std::string resultName);
    // accessSizeBytes 0: use operand size (unit tests / default). IR passes C width.
    void lvalueAssign(std::string operandName, std::string resultName, int accessSizeBytes = 0);
    // Narrow integral symbol to sizeBytes (1 or 4), zero- or sign-extending to 64 bits.
    void truncate(std::string operandName, int sizeBytes, bool isSigned);

    void procedureArgument(std::string argumentName);
    // memoryReturnDest: when non-empty, pass &dest in first integer arg reg (sret).
    void callProcedure(std::string procedureName, std::string memoryReturnDest = "");
    // Indirect call through a Value holding the function pointer.
    void callProcedureIndirect(std::string targetSymbolName, std::string memoryReturnDest = "");
    void returnFromProcedure(std::string returnSymbolName = "");
    // memoryReturn: true when Call used sret into returnSymbolName (explicit bit on Retrieve).
    void retrieveProcedureReturnValue(std::string returnSymbolName, bool memoryReturn = false);

    void xorCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void orCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void andCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName);

    void add(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void sub(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void mul(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void div(std::string leftOperandName, std::string rightOperandName, std::string resultName, bool unsignedDiv = false);
    void mod(std::string leftOperandName, std::string rightOperandName, std::string resultName, bool unsignedMod = false);

    void inc(std::string operandName, int amount = 1);
    void dec(std::string operandName, int amount = 1);

    void shl(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    // logical=true uses SHR (unsigned); false uses SAR (signed arithmetic).
    void shr(std::string leftOperandName, std::string rightOperandName, std::string resultName, bool logical = false);

    // Expression-form builtins: sizeBytes is 2/4/8 for bswap16/32/64.
    void bswap(std::string operandName, std::string resultName, int sizeBytes);
    void ctz(std::string operandName, std::string resultName);

    // System V AMD64 va_list: apPtr holds pointer to the 24-byte tag.
    // lastAddr empty => C23 form: use last named formal of the current variadic procedure.
    void vaStart(std::string apPtrName, std::string lastAddrName);
    // Fetch next arg of accessSizeBytes (1/2/4/8); floating uses fp_offset path.
    void vaArg(std::string apPtrName, std::string resultName, int accessSizeBytes, bool isFloating, bool isSigned);
    void vaCopy(std::string dstPtrName, std::string srcPtrName);

    void setScope(std::vector<Value> variables);

    // result = (&object)+offset (LeaObject) or (pointer_value)+offset (PointerValue).
    void fieldAddress(std::string baseName, int offsetBytes, std::string resultName,
            symbols::AddressBaseMode baseMode);
    // result = &base[index] with element stride; LeaObject LEAs the array object.
    void indexAddress(std::string baseName, std::string indexName, int elementSizeBytes, std::string resultName,
            symbols::AddressBaseMode baseMode);

private:
    void shiftBy(std::string leftOperandName, std::string rightOperandName, std::string resultName,
            std::string (InstructionSet::*emitShift)(const Register&) const);

    int pushProcedureArgument(Value& argument, int argumentOffset);
    // Delegates to type::object_abi::valueWords (at least one 8-byte word).
    int wordCount(const Value& symbol) const;
    void copyWords(Value& source, Value& destination);
    void loadWord(Value& symbol, int wordIndex, Register& dest);
    void storeWord(Register& source, Value& symbol, int wordIndex);
    // Store the low nbytes of source to [addr+offset] (1..8). Used so multi-word
    // copies do not write past getSizeInBytes() into a packed neighbor.
    void storeBytesAt(Register& source, Register& addr, int offset, int nbytes);

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

    // Frame home adjusted for RSP movement (stack arg pushes). Globals unchanged.
    Address homeAfterSpDelta(const Address& home, int spDeltaBytes) const;
    // Mov from memory home, applying spDelta for RSP-relative slots.
    void emitLoadFromHome(Value& symbol, Register& dest, int spDeltaBytes = 0);
    MemoryOperand memoryOperand(const Address& address) const;
    MemoryOperand memoryOperand(const Value& symbol) const;
    void emitLoad(Value& symbol, Register& dest);
    void emitStore(Register& source, Value& symbol);
    void bindResult(Register& reg, Value& result);
    // Park v in xmmIndex at dest width: int via cvtsi2ss/sd, float via movd/movq.
    void loadValueToXmm(Value& v, int xmmIndex, SseWidth destWidth);
    void gprToXmm(const Register& gpr, int xmmIndex, SseWidth width);
    void xmmToGpr(int xmmIndex, Register& gpr, SseWidth width);
    void emitFloatingBinary(Value& left, Value& right, Value& result, SseBin op);
    bool tryNumericAssignConvert(Value& operand, Value& result);

    Register& get64BitRegister();
    Register& get64BitRegisterExcluding(Register& registerToExclude);
    Register& get64BitRegisterExcluding(const std::vector<Register*>& registersToExclude);
    Register& get64BitRegisterExcluding(Register& registerToExclude,
            const std::vector<Register*>& registersToExclude);
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
    // Source-order call args; GP/xmm/stack classified at emit (SysV).
    std::vector<Value*> argumentSequence;

    // Emit integer/float register args and stack overflow for a call (pure SysV:
    // ints pack in GP, floats only in xmm0..xmm7). Returns stack bytes pushed
    // (for post-call cleanup); sets AL float count via emit.
    int emitCallArguments(std::size_t firstReg);

    // Callee for emitCall: either a named procedure or a Value holding a function pointer.
    struct CallTarget {
        enum class Kind { Named, Indirect } kind;
        std::string name; // procedure name or indirect target symbol

        static CallTarget named(std::string procedureName) {
            return CallTarget { Kind::Named, std::move(procedureName) };
        }
        static CallTarget indirect(std::string targetSymbol) {
            return CallTarget { Kind::Indirect, std::move(targetSymbol) };
        }
    };

    // Shared call emission: sret setup -> args -> rematerialize sret -> call -> stack cleanup.
    void emitCall(const CallTarget& target, const std::string& memoryReturnDest);

    // True if startProcedure has defined this name in the current assembly (local call, not PLT/GOT).
    bool isDefinedProcedure(const std::string& name) const;
    // Direct call: local symbol via call, external via callPlt (PIE).
    void emitNamedCall(const std::string& procedureName);

    int localVariableStackSize { 0 };
    // Index of the body quadruple currently being emitted (see Value::lastUseOrdinal).
    int instructionOrdinal { 0 };
    // Callee sret: name of the local holding the hidden return pointer (empty if none).
    std::string sretSymbolName;
    // True while emitting a variadic procedure (push save areas in prologue, pop on return).
    bool procedureIsVariadic { false };
    // Last named formal of the current procedure (for C23 va_start(ap) without last arg).
    std::string lastNamedFormalName;
    SysVVaStartState vaStartState;
    // Procedure names defined in this assembly unit (for call vs callPlt / lea vs loadGot).
    std::set<std::string> definedProcedures;
};

} // namespace codegen

#endif // STACKMACHINE_H_
