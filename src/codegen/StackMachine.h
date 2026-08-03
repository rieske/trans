#ifndef STACKMACHINE_H_
#define STACKMACHINE_H_

#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <vector>

#include "Address.h"
#include "Instruction.h"
#include "InstructionSet.h"
#include "Amd64Registers.h"
#include "GlobalVariable.h"
#include "JumpCondition.h"
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

    void startProcedure(const Procedure& procedure);
    void endProcedure();

    // After each body quadruple: drop dead expression temps from registers without
    // spilling them (their stack slots may already be reused by later temps).
    void finishInstruction();

    void label(std::string name);
    void jump(JumpCondition jumpCondition, std::string label);

    void compare(std::string leftSymbolName, std::string rightSymbolName, bool signedRel = true);
    void zeroCompare(std::string symbolName);

    void copyPart(std::string sourceName, std::string destName, int byteOffset);

    void addressOf(std::string operandName, std::string resultName);
    void functionAddress(std::string functionName, std::string resultName);
    // Address of a pool/data label (string constants); always local lea, not GOT.
    void assignLabelAddress(std::string label, std::string resultName);
    void dereference(std::string operandName, std::string resultName);

    void unaryMinus(std::string operandName, std::string resultName);
    void bitwiseNot(std::string operandName, std::string resultName);

    void assign(std::string operandName, std::string resultName);
    void widenInteger(std::string operandName, std::string resultName, bool signHighWord);
    void assignConstant(std::string constant, std::string resultName, std::string highWord = "");
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

    // Byte-swap: sizeBytes is 2/4/8 (rol+mask for 16).
    void bswap(std::string operandName, std::string resultName, int sizeBytes);
    // widthBytes 4 uses 32-bit BSF (ctz); otherwise 64-bit (ctzl/ctzll).
    void ctz(std::string operandName, std::string resultName, int widthBytes);
    void allocaBytes(std::string sizeName, std::string resultName);

    // System V AMD64 va_list: apPtr holds pointer to the 24-byte tag.
    void vaStart(std::string apPtrName);
    // Fetch next arg; eightbyte class and gprExtend come from the result Value.
    void vaArg(std::string apPtrName, std::string resultName);
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
            std::string (InstructionSet::*emitShift)(const Register&, GprWidth) const);

    // Delegates to type::object_abi::valueWords (at least one 8-byte word).
    int wordCount(const Value& symbol) const;
    void copyWords(Value& source, Value& destination);
    void loadWord(Value& symbol, int wordIndex, Register& dest, int spDelta = 0,
            std::vector<Register*> extraExclude = {});
    void storeWord(Register& source, Value& symbol, int wordIndex,
            std::vector<Register*> extraExclude = {});
    void copyFromPointer(Register& ptr, Value& dest);
    void copyToPointer(Value& src, Register& ptr, int spDelta = 0,
            std::vector<Register*> extraExclude = {});
    void copyBytes(Register& srcBase, Register& destBase, int n,
            const std::vector<Register*>& extraExclude = {});
    void loadEightbyteToXmm(Value& symbol, int eightbyte, int xmmIndex, int spDelta,
            const std::vector<Register*>& exclude);
    void storeEightbyteFromXmm(int xmmIndex, Value& symbol, int eightbyte,
            const std::vector<Register*>& exclude);
    Register& integerReturnReg(int integerIndex);
    std::vector<Register*> integerReturnRegs();
    void loadVaArgPiece(Register& addr, int byteOffset, Value& result, int eightbyte, Register& wordReg,
            const std::vector<Register*>& extraExclude = {});
    void alignAddressUp(Register& addr, int align, const std::vector<Register*>& live);
    void leaFrameOrGlobal(Value& symbol, Register& dest, int spDelta);
    // Store the low nbytes of source to [addr+offset] (1..8). Used so multi-word
    // copies do not write past getSizeInBytes() into a packed neighbor.
    void storeBytesAt(Register& source, Register& addr, int offset, int nbytes,
            std::vector<Register*> extraExclude = {});

    enum class WideIntegerOp { Add, Sub, And, Or, Xor };
    enum class WideShiftOp { Left, ArithmeticRight, LogicalRight };
    enum class X87Op { Add, Sub, Mul, Div };
    void emitIntegerBinary(Value& left, Value& right, Value& result, WideIntegerOp wide,
            std::string (InstructionSet::*regOp)(const Register&, const Register&, GprWidth) const);
    void emitIntegerDiv(Value& left, Value& right, Value& result, bool unsignedOp, bool remainder);
    Register& copyToNewRegister(Value& src);
    void adjust(std::string operandName, int amount, bool plus);
    bool isMultiWord(const Value& v) const;
    bool isWideInteger(const Value& v) const;
    void emitFloatingOrX87Binary(Value& left, Value& right, Value& result, SseBin op, X87Op x87);
    bool tryComplexBinary(Value& left, Value& right, Value& result, X87Op op);
    bool tryComplexUnaryMinus(Value& operand, Value& result);
    bool tryComplexCompare(Value& left, Value& right);
    bool tryComplexZeroCompare(Value& symbol);
    bool tryComplexAssignConvert(Value& operand, Value& result);
    void emitComplexBinary(Value& left, Value& right, Value& result, X87Op op);
    void emitComplexUnaryMinus(Value& operand, Value& result);
    void emitComplexCompare(Value& left, Value& right);
    void emitComplexZeroCompare(Value& symbol);
    void loadX87At(Value& symbol, int byteOffset, int sizeBytes);
    void storeX87At(Value& symbol, int byteOffset, int sizeBytes);
    MemoryOperand partOperand(Value& symbol, int byteOffset);
    void emitComplexX87Load(Value& symbol);
    void emitComplexX87Store(Value& symbol);
    bool tryX87Binary(Value& left, Value& right, Value& result, X87Op op);
    void emitX87Binary(Value& left, Value& right, Value& result, X87Op op);
    void emitX87UnaryMinus(Value& operand, Value& result);
    void emitX87Compare(Value& left, Value& right, bool signedRel);
    void emitX87ZeroCompare(Value& symbol);
    void emitX87Convert(Value& operand, Value& result);
    void setCompareFlagsFromTernary(Register& acc, bool signedRel);
    bool tryWideIntegerBinary(Value& left, Value& right, Value& result, WideIntegerOp op);
    void emitGprBinary(Value& left, Value& right, Value& result, WideIntegerOp op);
    bool tryWideUnaryMinus(Value& operand, Value& result);
    bool tryWideUnaryNot(Value& operand, Value& result);
    bool tryWideCompare(Value& left, Value& right, bool signedRel);
    bool tryWideZeroCompare(Value& symbol);
    bool tryWideShift(Value& value, Value& count, Value& result, WideShiftOp op);
    void wideIntegerBinary(Value& left, Value& right, Value& result, WideIntegerOp op);
    void wideUnaryMinus(Value& operand, Value& result);
    void wideUnaryNot(Value& operand, Value& result);
    void wideCompare(Value& left, Value& right, bool signedRel);
    void wideZeroCompare(Value& symbol);
    void wideShift(Value& value, Value& count, Value& result, WideShiftOp op);

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

    // Prefer registered object homes; fall back to the frame spill slot from Value::index.
    Address addressOf(const Value& symbol) const;
    Address spillSlotAddress(const Value& symbol) const;
    // True if the operand should be read/written in memory (global home or no register).
    bool residesInMemory(const Value& symbol) const;
    void registerFrameHome(const std::string& name, Address address);
    // Load into dest without reg.assign; required for globals (Address-only homes).
    void loadWithoutBinding(Value& symbol, Register& dest);

    // Frame home adjusted for RSP movement (outgoing stack region). Globals unchanged.
    Address homeAfterSpDelta(const Address& home, int spDeltaBytes) const;
    // Mov from memory home, applying spDelta for RSP-relative slots.
    void emitLoadFromHome(Value& symbol, Register& dest, int spDeltaBytes = 0);
    // Load from mem using Classification.gprExtend (or SSE width for FLOATING).
    void emitClassifiedLoad(const MemoryOperand& mem, Register& dest, const Value& symbol);
    MemoryOperand memoryOperand(const Address& address) const;
    MemoryOperand memoryOperand(const Value& symbol) const;
    // Frame homes only; globals are handled by partOperand.
    MemoryOperand memoryOperandAt(const Value& symbol, int byteOffset) const;
    void emitLoad(Value& symbol, Register& dest);
    void emitStore(Register& source, Value& symbol);
    // Write n object bytes (1/2/4/8). Memory holds the C object, not a canonical qword.
    void storeObject(Register& source, const MemoryOperand& dest, int n);
    void storeObject(Register& source, Value& symbol);
    void bindResult(Register& reg, Value& result);
    GprWidth aluWidth(const Value& op) const;
    void signExtendIfNarrow(Register& reg, const Value& v, GprWidth width);
    void commitIntegral(Register& reg, Value& result, GprWidth width);
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

    // Emit register/stack args from SysV eightbyte classification. Returns
    // outgoing stack bytes to free after the call; sets AL SSE count.
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

    void createVaSaveHomes(int vaSaveBaseIndex, std::vector<std::string>& vaGpHome,
            std::vector<std::string>& vaXmmHome);
    void dumpVariadicSaveArea(const std::vector<std::string>& vaGpHome,
            const std::vector<std::string>& vaXmmHome);
    void loadVaListTagPointer(const std::string& apName, Register& dest);

    int localVariableStackSize { 0 };
    bool hasFrame_ { false };
    int frameSubAmount_ { 0 };
    // Index of the body quadruple currently being emitted (see Value::lastUseOrdinal).
    int instructionOrdinal { 0 };
    // Callee sret: name of the local holding the hidden return pointer (empty if none).
    std::string sretSymbolName;
    struct VariadicFrame {
        Address regSave;
        Address overflow;
        int namedGpOffset { 0 };
        int namedFpOffset { 0 };
    };
    std::optional<VariadicFrame> variadicFrame;
    int vaArgSeq { 0 };
    int wideLabel_ { 0 };
    // Procedure names defined in this assembly unit (for call vs callPlt / lea vs loadGot).
    std::set<std::string> definedProcedures;
};

} // namespace codegen

#endif // STACKMACHINE_H_
