#ifndef STACKMACHINE_H_
#define STACKMACHINE_H_

#include <deque>
#include <map>
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
#include "IrStringTable.h"
#include "JumpCondition.h"
#include "Value.h"
#include "Assembly.h"
#include "symbols/AddressPlan.h"
#include "types/ObjectAbi.h"

namespace codegen {

class StackMachine {
public:
    StackMachine(std::ostream* ostream, InstructionSet& instructions,
            Amd64Registers& registers, const IrStringTable& strings);
    StackMachine(const StackMachine&) = delete;
    StackMachine(StackMachine&&) = default;
    virtual ~StackMachine() = default;

    StackMachine& operator=(const StackMachine&) = delete;
    StackMachine& operator=(StackMachine&&) = default;

    void generatePreamble(const std::map<std::string, std::string>& constants,
            const std::vector<GlobalVariable>& globalVariables,
            const std::vector<std::string>& externalSymbols = {});

    void startProcedure(const Procedure& procedure);
    void endProcedure();
    void finishInstruction();

    void label(int name);
    void jump(JumpCondition jumpCondition, int label, bool signedRel = true);

    void compare(int leftSymbolName, int rightSymbolName, bool signedRel = true);
    void zeroCompare(int symbolName);

    void addressOf(int operandName, int resultName);
    void functionAddress(int functionName, int resultName);
    void dereference(int operandName, int lvalueName, int resultName);
    void indexAddress(int baseName, int indexName, int elementSizeBytes, int resultName,
            symbols::AddressBaseMode baseMode = symbols::AddressBaseMode::LeaObject);
    // Pointer value +/- integer: result = base +/- index * elementSizeBytes.
    void pointerOffset(int baseName, int indexName, int elementSizeBytes, int resultName,
            bool subtract);
    // Pointer - pointer: result = (left - right) / elementSizeBytes (element count).
    void pointerDifference(int leftName, int rightName, int elementSizeBytes, int resultName);
    void fieldAddress(int baseName, int offsetBytes, int resultName,
            symbols::AddressBaseMode baseMode = symbols::AddressBaseMode::LeaObject);
    void copyPart(int sourceName, int destName, int byteOffset);

    void unaryMinus(int operandName, int resultName);
    void unaryNot(int operandName, int resultName);

    void assign(int operandName, int resultName);
    void widenInteger(int operandName, int resultName, bool signHighWord);
    void assignConstant(int constant, int resultName, int highWord = kNoSymbol);
    void assignLabelAddress(int label, int resultName);
    void lvalueAssign(int operandName, int resultName);

    void procedureArgument(int argumentName);
    // memoryReturnDest < 0: no sret pointer in first integer arg reg.
    void callProcedure(int procedureName, int memoryReturnDest = kNoSymbol);
    // Indirect call through a Value holding the function pointer.
    void callProcedureIndirect(int targetSymbolName, int memoryReturnDest = kNoSymbol);
    void returnFromProcedure(int returnSymbolName = kNoSymbol);
    // memoryReturn: true when Call used sret into returnSymbolName.
    void retrieveProcedureReturnValue(int returnSymbolName, bool memoryReturn = false);

    // lastAddr == kNoSymbol => C23 form: last named formal of the current procedure.
    void vaStart(int apPtrName, int lastAddrName = kNoSymbol);
    void vaArg(int apPtrName, int resultName);
    void vaCopy(int dstPtrName, int srcPtrName);
    void vaEnd();
    void bswap(int operandName, int resultName, int widthBytes);
    void ctz(int operandName, int resultName, int widthBytes);
    void allocaBytes(int sizeName, int resultName);

    void xorCommand(int leftOperandName, int rightOperandName, int resultName);
    void orCommand(int leftOperandName, int rightOperandName, int resultName);
    void andCommand(int leftOperandName, int rightOperandName, int resultName);

    void add(int leftOperandName, int rightOperandName, int resultName);
    void sub(int leftOperandName, int rightOperandName, int resultName);
    void mul(int leftOperandName, int rightOperandName, int resultName);
    void div(int leftOperandName, int rightOperandName, int resultName,
            bool signedDiv = true);
    void mod(int leftOperandName, int rightOperandName, int resultName,
            bool signedDiv = true);

    // step: 1 for scalar ++/--; sizeof(*p) bytes for pointer ++/--.
    void inc(int operandName, int step = 1);
    void dec(int operandName, int step = 1);

    void shl(int leftOperandName, int rightOperandName, int resultName);
    void shr(int leftOperandName, int rightOperandName, int resultName,
            bool arithmetic);

    void setScope(std::vector<Value> variables);

    void registerDefinedProcedure(int procedureName);

private:
    bool isDefinedProcedure(int name) const;
    // Shared by indexAddress and pointerOffset: sign/zero-extend index into RAX, imul if stride != 1.
    void scaleIntegerIntoRax(Value& index, int elementSizeBytes);
    // LEA object home or load/mov pointer value into dest.
    void materializeBaseAddress(Value& base, symbols::AddressBaseMode baseMode, Register& dest);
    // result = base +/- index * elementSizeBytes (LEA object home or pointer value).
    void scaledBaseIndex(int baseName, int indexName, int elementSizeBytes, int resultName,
            symbols::AddressBaseMode baseMode, bool subtract);

    void shiftBy(int leftOperandName, int rightOperandName, int resultName,
            std::string (InstructionSet::*emitShift)(const Register&, int) const);

    // Shared call setup; then either call label or *reg.
    // Returns stack argument bytes to free after the call.
    int emitCallArguments(std::size_t firstReg = 0);
    void emitCall(bool indirect, int target, int memoryReturnDest);
    void leaFrameOrGlobal(Value& symbol, Register& dest, int spDelta);
    void loadWord(Value& symbol, int wordIndex, Register& dest, int spDelta = 0,
            std::vector<Register*> extraExclude = {});
    void bindGprExtended(Value& symbol);
    void storeWord(Register& source, Value& symbol, int wordIndex);
    void copyWords(Value& source, Value& destination);
    // Exact-size copies (not ceil-to-word). Required for packed objects and sret.
    void copyFromPointer(Register& ptr, Value& dest);
    void copyToPointer(Value& src, Register& ptr, int spDelta = 0,
            std::vector<Register*> extraExclude = {});
    void copyBytes(Register& srcBase, Register& destBase, int n,
            const std::vector<Register*>& extraExclude = {});
    void emitIntegerDivide(Value& left, Value& right, bool signedDiv);
    void loadEightbyteToXmm(Value& symbol, int eightbyte, int xmmIndex, int spDelta,
            const std::vector<Register*>& exclude);
    void storeEightbyteFromXmm(int xmmIndex, Value& symbol, int eightbyte,
            const std::vector<Register*>& exclude);
    Register& integerReturnReg(int integerIndex);
    std::vector<Register*> integerReturnRegs();
    void loadVaArgPiece(Register& addr, int byteOffset, Value& result, int eightbyte, Register& wordReg);
    void alignAddressUp(Register& addr, int align, const std::vector<Register*>& live);
    // Park v in xmmIndex at dest width: int via cvtsi2ss/sd, float via movd/movq.
    void loadValueToXmm(Value& v, int xmmIndex, bool destFloat32);
    void gprToXmm(const Register& gpr, int xmmIndex, bool destFloat32);
    void xmmToGpr(int xmmIndex, Register& gpr, bool destFloat32);
    void emitFloatingBinary(Value& left, Value& right, Value& result,
            std::string (InstructionSet::*ssOp)(int, int) const,
            std::string (InstructionSet::*sdOp)(int, int) const);
    bool involvesFloating(const Value& left, const Value& right, const Value& result) const;
    bool tryNumericAssignConvert(Value& operand, Value& result);

    enum class X87Op { Add, Sub, Mul, Div };
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
    void emitFloatingOrX87Binary(Value& left, Value& right, Value& result,
            std::string (InstructionSet::*ssOp)(int, int) const,
            std::string (InstructionSet::*sdOp)(int, int) const,
            X87Op op);
    bool tryX87Binary(Value& left, Value& right, Value& result, X87Op op);
    void emitX87Binary(Value& left, Value& right, Value& result, X87Op op);
    void emitX87UnaryMinus(Value& operand, Value& result);
    void emitX87Compare(Value& left, Value& right, bool signedRel);
    void emitX87ZeroCompare(Value& symbol);
    void emitX87Convert(Value& operand, Value& result);
    void setCompareFlagsFromTernary(Register& acc, bool signedRel);

    enum class WideIntegerOp { Add, Sub, And, Or, Xor };
    enum class WideShiftOp { Left, ArithmeticRight, LogicalRight };
    bool isMultiWord(const Value& v) const;
    bool isWideInteger(const Value& v) const;
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
    Value& resolve(int id);
    const std::string& text(int id) const;
    void put(std::deque<Value>& storage, std::vector<Value*>& byId, Value value);
    void registerFrameHome(int id, Address address);

    // Prefer registered object homes; fall back to the frame spill slot from Value::index.
    Address addressOf(const Value& symbol) const;
    Address spillSlotAddress(const Value& symbol) const;
    // True if the operand should be read/written in memory (global home or no register).
    bool residesInMemory(const Value& symbol) const;

    // Load into dest without reg.assign; required for globals (Address-only homes).
    void loadWithoutBinding(Value& symbol, Register& dest);
    MemoryOperand memoryOperand(const Address& address) const;
    MemoryOperand memoryOperand(const Value& symbol) const;
    MemoryOperand memoryOperandAt(const Value& symbol, int byteOffset) const;
    void emitComplexX87Load(Value& symbol);
    void emitComplexX87Store(Value& symbol);
    void emitLoad(Value& symbol, Register& dest);
    void emitStore(Register& source, Value& symbol);
    void bindResult(Register& reg, Value& result);
    int promotedBytes(const Value& value) const;
    MemoryOperand homeOperand(const Value& symbol, int spDelta = 0) const;
    void emitGprExtend(type::sysv::GprExtend ext, int size, const MemoryOperand& source, Register& dest);
    void loadPromotedFrom(const MemoryOperand& source, const Value& typed, Register& dest);
    void loadPromoted(Value& symbol, Register& dest, int spDelta = 0);
    void storeObject(Register& source, const MemoryOperand& dest, int n);
    void storeObject(Register& source, Value& symbol, int spDelta = 0);
    void copyToRegister(Value& symbol, Register& dest);
    void canonicalize(Register& reg, Value& symbol);
    Register& materialize(Value& symbol);
    Register& materializeExcluding(Value& symbol, Register& exclude);
    void stepLvalue(Value& operand, bool increment, int step);

    void dumpVariadicSaveArea(const std::vector<int>& vaGpHome,
            const std::vector<int>& vaXmmHome);
    void createVaSaveHomes(int vaSaveBaseIndex, const std::vector<int>& vaGpHome,
            const std::vector<int>& vaXmmHome);
    void loadVaListTagPointer(int apName, Register& dest);

    Register& get64BitRegister();
    Register& get64BitRegisterExcluding(Register& registerToExclude);
    Register& get64BitRegisterExcluding(const std::vector<Register*>& exclude);
    Register& getCounterRegister();
    Register& assignRegisterTo(Value& symbol);
    Register& assignRegisterExcluding(Value& symbol, Register& registerToExclude);

    Assembly assembly;
    InstructionSet* instructionSet;

    Amd64Registers* registers;
    std::vector<Register*> calleeSavedRegisters;

    const IrStringTable& strings_;
    // Pointer-stable storage; resolve indexes by intern id.
    std::deque<Value> scopeStorage;
    std::vector<Value*> scopeById;
    std::deque<Value> globalStorage;
    std::vector<Value*> globalById;
    // Object homes (Address); globals and frame slots, keyed by intern id.
    std::map<int, Address> globalHomes;
    std::map<int, Address> frameHomes;
    // Source-order call args; GP/xmm/stack classified at emit (SysV).
    std::vector<Value*> argumentSequence;

    std::set<int> definedProcedures;
    int sretId_ { kNoSymbol };

    bool hasFrame_ { false };
    type::object_abi::FrameLayout frameLayout_ {};
    int instructionOrdinal { 0 };

    struct VariadicFrame {
        Address regSave;
        Address overflow;
        int lastNamedFormal { kNoSymbol };
        bool lastFormalOnStack { false };
        int namedGpOffset { 0 };
        int namedFpOffset { 0 };
    };
    std::optional<VariadicFrame> variadicFrame;
    int vaArgSeq { 0 };
    int wideLabel_ { 0 };
};

} // namespace codegen

#endif // STACKMACHINE_H_
