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
#include "Assembly.h"
#include "symbols/AddressPlan.h"
#include "types/ObjectAbi.h"

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
            const std::vector<std::string>& externalSymbols = {});

    void startProcedure(const Procedure& procedure);
    void endProcedure();

    void label(std::string name);
    void jump(JumpCondition jumpCondition, std::string label, bool signedRel = true);

    void compare(std::string leftSymbolName, std::string rightSymbolName, bool signedRel = true);
    void zeroCompare(std::string symbolName);

    void addressOf(std::string operandName, std::string resultName);
    void functionAddress(std::string functionName, std::string resultName);
    void dereference(std::string operandName, std::string lvalueName, std::string resultName);
    void indexAddress(std::string baseName, std::string indexName, int elementSizeBytes, std::string resultName,
            symbols::AddressBaseMode baseMode = symbols::AddressBaseMode::LeaObject);
    // Pointer value +/- integer: result = base +/- index * elementSizeBytes.
    void pointerOffset(std::string baseName, std::string indexName, int elementSizeBytes, std::string resultName,
            bool subtract);
    // Pointer - pointer: result = (left - right) / elementSizeBytes (element count).
    void pointerDifference(std::string leftName, std::string rightName, int elementSizeBytes, std::string resultName);
    void fieldAddress(std::string baseName, int offsetBytes, std::string resultName,
            symbols::AddressBaseMode baseMode = symbols::AddressBaseMode::LeaObject);
    void copyPart(std::string sourceName, std::string destName, int byteOffset);

    void unaryMinus(std::string operandName, std::string resultName);
    void unaryNot(std::string operandName, std::string resultName);

    void assign(std::string operandName, std::string resultName);
    void widenInteger(std::string operandName, std::string resultName, bool signHighWord);
    void assignConstant(std::string constant, std::string resultName, std::string highWord = "");
    void assignLabelAddress(std::string label, std::string resultName);
    void lvalueAssign(std::string operandName, std::string resultName);

    void procedureArgument(std::string argumentName);
    // memoryReturnDest: when non-empty, pass &dest in first integer arg reg (sret).
    void callProcedure(std::string procedureName, std::string memoryReturnDest = "");
    // Indirect call through a Value holding the function pointer.
    void callProcedureIndirect(std::string targetSymbolName, std::string memoryReturnDest = "");
    void returnFromProcedure(std::string returnSymbolName = "");
    // memoryReturn: true when Call used sret into returnSymbolName.
    void retrieveProcedureReturnValue(std::string returnSymbolName, bool memoryReturn = false);

    // lastAddr empty => C23 form: last named formal of the current procedure.
    void vaStart(std::string apPtrName, std::string lastAddrName);
    void vaArg(std::string apPtrName, std::string resultName);
    void vaCopy(std::string dstPtrName, std::string srcPtrName);
    void vaEnd();
    void bswap(std::string operandName, std::string resultName, int widthBytes);
    void ctz(std::string operandName, std::string resultName, int widthBytes);
    void allocaBytes(std::string sizeName, std::string resultName);

    void xorCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void orCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void andCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName);

    void add(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void sub(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void mul(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void div(std::string leftOperandName, std::string rightOperandName, std::string resultName,
            bool signedDiv = true);
    void mod(std::string leftOperandName, std::string rightOperandName, std::string resultName,
            bool signedDiv = true);

    // step: 1 for scalar ++/--; sizeof(*p) bytes for pointer ++/--.
    void inc(std::string operandName, int step = 1);
    void dec(std::string operandName, int step = 1);

    void shl(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    void shr(std::string leftOperandName, std::string rightOperandName, std::string resultName,
            bool arithmetic);

    void setScope(std::vector<Value> variables);

    void registerDefinedProcedure(std::string procedureName);

private:
    bool isDefinedProcedure(const std::string& name) const;
    // Shared by indexAddress and pointerOffset: sign/zero-extend index into RAX, imul if stride != 1.
    void scaleIntegerIntoRax(Value& index, int elementSizeBytes);
    // LEA object home or load/mov pointer value into dest.
    void materializeBaseAddress(Value& base, symbols::AddressBaseMode baseMode, Register& dest);
    // result = base +/- index * elementSizeBytes (LEA object home or pointer value).
    void scaledBaseIndex(std::string baseName, std::string indexName, int elementSizeBytes, std::string resultName,
            symbols::AddressBaseMode baseMode, bool subtract);

    void shiftBy(std::string leftOperandName, std::string rightOperandName, std::string resultName,
            std::string (InstructionSet::*emitShift)(const Register&, int) const);

    // Shared call setup; then either call label or *reg.
    // Returns stack argument bytes to free after the call.
    int emitCallArguments(std::size_t firstReg = 0);
    void emitCall(bool indirect, const std::string& target, const std::string& memoryReturnDest);
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
    Value& resolve(const std::string& name);

    // Prefer registered object homes; fall back to the frame spill slot from Value::index.
    Address addressOf(const Value& symbol) const;
    Address spillSlotAddress(const Value& symbol) const;
    // True if the operand should be read/written in memory (global home or no register).
    bool residesInMemory(const Value& symbol) const;
    void registerFrameHome(const std::string& name, Address address);
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

    void dumpVariadicSaveArea(const std::vector<std::string>& vaGpHome,
            const std::vector<std::string>& vaXmmHome);
    void createVaSaveHomes(int vaSaveBaseIndex, std::vector<std::string>& vaGpHome,
            std::vector<std::string>& vaXmmHome);
    void loadVaListTagPointer(const std::string& apName, Register& dest);

    Register& get64BitRegister();
    Register& get64BitRegisterExcluding(Register& registerToExclude);
    Register& get64BitRegisterExcluding(const std::vector<Register*>& exclude);
    Register& getCounterRegister();
    Register& assignRegisterTo(Value& symbol);
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

    std::set<std::string> definedProcedures;
    std::string sretSymbolName;

    bool hasFrame_ { false };
    type::object_abi::FrameLayout frameLayout_ {};

    struct VariadicFrame {
        Address regSave;
        Address overflow;
        std::string lastNamedFormal;
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
